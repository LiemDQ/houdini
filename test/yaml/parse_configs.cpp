#include "gtest/gtest.h"
#include "estate/hw/parse_ni.hpp"
#include "estate/hw/parse_hw_yaml.hpp"

using namespace estate::hw::nidaqmx;
using namespace estate::hw;

// ============ ni config ================

TEST(YamlParseTest, simpleNIDevicesParse) {
  YAML::Node raw = YAML::Load(
      "- name: device_name_here\n"
      "  modules:\n"
      "  - name: module_name_here\n"
      "    type: module_type_here\n"
      "    task: module_task_here\n"
      "    channels:\n"
      "    - name: channel_name_here\n"
      "      hardware: hardware_type_here\n");

  auto devices = parseNIDAQmxConfigDevices(raw);
  ASSERT_EQ(devices.size(), 1);
  ASSERT_EQ(devices[0].name, "device_name_here");
  auto modules = devices[0].modules;
  ASSERT_EQ(modules.size(), 1);
  ASSERT_EQ(modules[0].name, "module_name_here");
  ASSERT_EQ(modules[0].type, "module_type_here");
  ASSERT_EQ(modules[0].task, "module_task_here");
  auto channels = modules[0].channels;
  ASSERT_EQ(channels.size(), 1);
  ASSERT_EQ(channels[0].name, "channel_name_here");
  ASSERT_EQ(channels[0].hardware, "hardware_type_here");
}

TEST(YamlParseTest, simpleTasksParse) {
  YAML::Node raw = YAML::Load(
      "- name: task_name_here\n"
      "  sample_rate: 1.0\n"
      "  timing: timing_type_here\n"
      "  type: task_type_here\n"
      );

  auto tasks = parseNIDAQmxConfigTasks(raw);
  ASSERT_EQ(tasks.size(), 1);
  ASSERT_EQ(tasks[0].name, "task_name_here");
  ASSERT_EQ(tasks[0].sample_rate, 1);
  ASSERT_EQ(tasks[0].timing, "timing_type_here");
  ASSERT_EQ(tasks[0].type, "task_type_here");
}

// ============ sensors ================

TEST(YamlParseTest, basicDeviceParse) {
  YAML::Node raw = YAML::Load(
      "- name: sensor_name_here\n"
      "  serial_no: serial_number_here\n"
      "  signal: other\n" // basic device
      "  description: description_here\n"
      "  scale:\n"
      "    name: scale_name_here\n"
      "    type: scale_type_here\n"
      "    prescaled: [0.0, 1.0, 2.0]\n"
      "    prescaled_unit: prescaled_units_here\n"
      "    scaled: [1.0, 2.0, 3.0]\n"
      "    scaled_unit: scaled_units_here\n");

  auto parse_result = parseHardware(raw);
  auto devices = parse_result.devices();
  ASSERT_EQ(devices.size(), 1);


  const auto& device = devices.at("sensor_name_here");
  const auto& basic_device = std::get<BaseDeviceInfo>(device);

  ASSERT_EQ(basic_device.name, "sensor_name_here");
  ASSERT_EQ(basic_device.signal, "other");
  ASSERT_EQ(*basic_device.serial_number, "serial_number_here");
  ASSERT_EQ(*basic_device.description, "description_here");
  auto bd_scale = *basic_device.scale;
  ASSERT_EQ(bd_scale.name, "scale_name_here");
  ASSERT_EQ(bd_scale.type, "scale_type_here");
  ASSERT_EQ(bd_scale.prescaled_units, "prescaled_units_here");
  ASSERT_EQ(bd_scale.scaled_units, "scaled_units_here");
  {
    float i = 0;
    auto iter = bd_scale.prescaled_begin;
    while (iter != bd_scale.prescaled_end) {
      ASSERT_EQ(*iter++, i++);
    }
  }
  {
    float i = 1;
    auto iter = bd_scale.scaled_begin;
    while (iter != bd_scale.scaled_end) {
      ASSERT_EQ(*iter++, i++);
    }
  }
}
