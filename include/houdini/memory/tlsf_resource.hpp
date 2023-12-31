#pragma once

#include <cstddef>
#include <cassert>
#include <memory_resource>
#include <utility>

namespace houdini {
namespace memory {

#if INTPTR_MAX == INT64_MAX
#define TLSF_64BIT
#elif INTPTR_MAX == INT32_MAX
// 32 bit
#else
#error Unsupported bitness architecture for TLSF allocator.
#endif

#define TLSF_CAST(t, exp) ((t)(exp))

using tlsfptr_t = std::ptrdiff_t;

class tlsf_resource : public std::pmr::memory_resource {
    public:
        static constexpr std::size_t DEFAULT_POOL_SIZE = 1024*1024;

        explicit tlsf_resource(std::size_t size){
            initialize(size);
        };

        explicit tlsf_resource() noexcept: memory_pool(nullptr){
            initialize(DEFAULT_POOL_SIZE);
        }
        
        tlsf_resource(const tlsf_resource& resource) noexcept 
        : memory_pool(resource.memory_pool), pool_size(resource.pool_size) {}
        
        ~tlsf_resource(){
            //NOTE: make sure the tlsf resource outlives any objects whose memory 
            //is allocated by it! Otherwise this will result in dangling pointers.
            if (this->memory_pool){
                free((void*)(this->memory_pool));
                memory_pool = nullptr;
            }
        }

    private:

        /**
         * TLSF block header. 
         * According to the TLSF specification:
         * - the prev_phys_block fields is only valid if the previous block is free
         * - the prev_phys_block is actually stored at the end of the previous block. This arrangement simplifies the implementation.
         * - The next_free and prev_free are only valid if the block is free.
         */
        struct block_header {
            block_header* prev_phys_block;
            std::size_t size;
            
            block_header* next_free;
            block_header* prev_free;

            /**
             * Block sizes are always a multiple of 4. 
             * The two least significant bits of the size field are used to store the block status
             *  bit 0: whether the block is busy or free
             *  bit 1: whether the previous block is busy or free
             * 
             * Block overhead: 
             * The only overhead exposed during usage is the size field. The previous_hys_block field is technically stored 
             * inside the previous block.
             */
            static constexpr std::size_t block_header_free_bit = 1 << 0;
            static constexpr std::size_t block_header_prev_free_bit = 1 << 1;
            static constexpr std::size_t block_header_overhead = sizeof(std::size_t);

            std::size_t get_size() const { 
                //must filter out last two bits
                return this->size & ~(this->block_header_free_bit | this->block_header_prev_free_bit);
            }

            void set_size(std::size_t new_size){
                const std::size_t old_size = this->size;
                //must retain the last two bits regardless of the new size
                this->size = new_size | (old_size & (this->block_header_free_bit | this->block_header_prev_free_bit));
            };

            bool is_last() const { return this->get_size() == 0;}
            bool is_free() const { return TLSF_CAST(int, this->size & this->block_header_free_bit);}
            bool is_prev_free() const { return TLSF_CAST(int, this->size & this->block_header_prev_free_bit);}


            static block_header* from_void_ptr(const void* ptr){
                //note the intermediate conversion to char ptr is to get 1-byte displacements.
                return TLSF_CAST(block_header*, TLSF_CAST(unsigned char*, ptr)-block_start_offset);
            }

            void* to_void_ptr() const {
                return TLSF_CAST(void*, TLSF_CAST(unsigned char*, this) + block_start_offset);
            }
            /*Returns a block pointer offset from the passed ptr by the size given*/
            static block_header* offset_to_block(const void* ptr, std::size_t blk_size){
                //possiblyt could remove ptr and use block->to_void_ptr instead.
                return TLSF_CAST(block_header*, TLSF_CAST(tlsfptr_t, ptr)+blk_size);
            }

            block_header* get_next(){
                block_header* next = this->offset_to_block(this->to_void_ptr(), this->get_size()-tlsf_resource::block_start_offset);
                assert(!this->is_last());
                return next;
            }

            block_header* link_next(){
                block_header* next = this->get_next();
                next->prev_phys_block = this;
                return next;
            }

            void mark_as_free() {
                block_header* next = this->link_next();
                next->set_prev_free();
                this->set_free();
            }

            void mark_as_used(){
                block_header* next = this->get_next();
                next->set_prev_used();
                this->set_used();
            }
            
            void set_free() {this->size |= this->block_header_free_bit; }
            void set_used() {this->size &= ~this->block_header_free_bit; }
            void set_prev_free() { this->size |= this->block_header_free_bit; }
            void set_prev_used() { this->size &= ~this->block_header_prev_free_bit; }
        /* User data starts after the size field in a used block */
        };

        static constexpr std::size_t block_start_offset = offsetof(block_header, size) + sizeof(size_t);

        #ifdef TLSF_64BIT
        // all allocation sizes are aligned to 8 bytes
        static constexpr int align_size_log2 = 3;
        static constexpr int fl_index_max = 32; //this means the largest block we can allocate is 2^32 bytes
        #else
        // all allocation sizes are aligned to 4 bytes
        static constexpr int align_size_log2 = 2;
        static constexpr int fl_index_max = 30;
        #endif

        static constexpr int align_size = (1 << align_size_log2);

        // log2 of number of linear subdivisions of block sizes
        // values of 4-5 typical
        static constexpr int sl_index_count_log2 = 5;

        /**
         * Allocations of sizes up to (1 << fl_index_max) are supported.
         * Because we linearly subdivide the second-level lists and the minimum size block
         * is N bytes, it doesn't make sense ot create first-level lists for sizes smaller than
         * sl_index_count * N or (1 << (sl_index_count_log2 + log(N))) bytes, as we will be trying to split
         * size ranges into more slots than we have available. We calculate the minimum threshold
         * size, and place all blocks below that size into the 0th first-level list. 
         */
        static constexpr int sl_index_count = (1 << sl_index_count_log2);
        static constexpr int fl_index_shift = (sl_index_count_log2 + align_size_log2);
        static constexpr int fl_index_count = (fl_index_max - fl_index_shift + 1);
        static constexpr int small_block_size = (1 << fl_index_shift);

        const inline std::size_t tlsf_size(){
            return sizeof(*this);
        }

        static constexpr std::size_t block_size_min = sizeof(block_header) - sizeof(decltype(std::declval<block_header>().prev_phys_block));
        static constexpr std::size_t block_size_max = static_cast<std::size_t>(1) << fl_index_max;
        static constexpr std::size_t pool_overhead = 2*block_header::block_header_overhead;
        static constexpr std::size_t tlsf_alloc_overhead = block_header::block_header_overhead;
    
        block_header block_null;
        unsigned int fl_bitmap;
        unsigned int sl_bitmap[fl_index_count];

        block_header* blocks[fl_index_count][sl_index_count];

        void initialize(std::size_t size);
        
        static constexpr std::size_t alignUp(std::size_t x, std::size_t align);
        static constexpr std::size_t alignDown(std::size_t x, std::size_t align);
        static void* alignPtr(const void* ptr, std::size_t align);
        char* createMemoryPool(std::size_t bytes, char* pool);
        

        
        void* mallocPool(std::size_t size);
        void freePool(void* ptr);
        void* reallocPool(void* ptr, std::size_t size);
        
        //TODO:
        // Implement memalign and placement new functionality for this memory resource.
        // What does it mean to have placement new and alignment when the block partitioning 
        // is done by the underlying implementation anyway? Can we overwrite the same block?
        void* memalign(std::size_t align, std::size_t size);

        void* do_allocate(std::size_t bytes, std::size_t alignment) override;
        void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override;

        bool do_is_equal(const tlsf_resource& other) const noexcept;
        bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

        /**
         * TLSF utility functions 
         * Based on the implementation described in this paper:
         * http://www.gii.upv.es/tlsf/files/spe_2008.pdf
         */

        static void mappingSearch(std::size_t size, int* fli, int* sli);
        static void mappingInsert(std::size_t size, int* fli, int* sli);
        
        void removeFreeBlock(block_header* block, int fl, int sl);
        void insertFreeBlock(block_header* block, int fl, int sl);        
        block_header* searchSuitableBlock(int* fli, int* sli);

        void blockRemove(block_header* block);
        void blockInsert(block_header* block);
        static bool blockCanSplit(block_header* block, std::size_t size);
        static block_header* blockSplit(block_header* block, std::size_t size);
        static std::size_t adjustRequestSize(std::size_t size, std::size_t align);

        void trimFree(block_header* block, std::size_t size);
        void trimUsed(block_header* block, std::size_t size);

        block_header* trimFreeLeading(block_header* block, std::size_t size);
        static block_header* blockCoalesce(block_header* prev, block_header* block);
        block_header* mergePrev(block_header* block);
        block_header* mergeNext(block_header* block);
        block_header* locateFree(std::size_t size);
        void* prepareUsed(block_header* block, std::size_t size);

        char* memory_pool;
        std::size_t pool_size;
        std::pmr::memory_resource* upstream;
};


} //namespace memory
} // namespace houdini