/*
 * Copyright (c) 2020-2021 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "CBuildIntegTestEnv.h"

using namespace std;

error_code ec;
string scripts_folder   = string(TEST_FOLDER) + "scripts";
string testinput_folder = string(TEST_FOLDER) + "testinput";
string cbuildgen_bin    = string(CBUILDGEN_BIN);
string testout_folder   = fs::current_path(ec).append("testoutput").generic_string();
string testdata_folder  = testout_folder + "/testdata";
string examples_folder  = testdata_folder + "/Examples";

std::string CBuildIntegTestEnv::ci_installer_path;
std::string CBuildIntegTestEnv::ac6_toolchain_path;

void RunScript(const string& script, string arg) {
  int ret_val;

  ASSERT_EQ(true, fs::exists(scripts_folder + "/" + script, ec))
    << "error: " << script << " not found";

  string cmd = "cd " + scripts_folder + " && " + SH + " \"./"
    + script + (arg.empty() ? "" : " " + arg) + "\"";
  ret_val = system(cmd.c_str());
  ASSERT_EQ(ret_val, 0);
}

// Workaround for fs::remove_all
// Old gcc has a bug with remove_all
void RemoveDir(fs::path path) {
  string cmd, rmPath = path.string();
  int ret_val;
  size_t s = 0;

  while ((s = rmPath.find("\\", s)) != string::npos) {
    rmPath.replace(s, 1, "/");
  }

  cmd = "bash -c \"rm -rf \\\"" + rmPath + "\\\"\"";
  ret_val = system(cmd.c_str());
  ASSERT_EQ(ret_val, 0);
}

void CBuildIntegTestEnv::SetUp() {
  error_code ec;
  fs::create_directories(testout_folder, ec);
  fs::create_directories(testdata_folder, ec);

  testinput_folder = fs::canonical(testinput_folder, ec).generic_string();
  testdata_folder  = fs::canonical(testdata_folder, ec).generic_string();

  // Copy test data from input test folder
  fs::copy(fs::path(testinput_folder), fs::path(testdata_folder), fs::copy_options::recursive, ec);

  examples_folder  = fs::canonical(examples_folder, ec).generic_string();
  scripts_folder   = fs::canonical(scripts_folder, ec).generic_string();
  testinput_folder = fs::canonical(testinput_folder, ec).generic_string();

  RunScript("setup_test.sh", "--binary=" + cbuildgen_bin + " --output=" + testout_folder);
  RunScript("download_packs.sh", testout_folder);

  ci_installer_path = CrossPlatformUtils::GetEnv("CI_CBUILD_INSTALLER");

  // Read AC6 toolchain path
  string ac6ToolchainFilePath = testout_folder + "/cbuild/etc/AC6.6.16.0.cmake";
  ASSERT_TRUE(fs::exists(ac6ToolchainFilePath));

  ifstream file(ac6ToolchainFilePath);
  string line;
  while (getline(file, line)) {
    if (string::npos != line.find("TOOLCHAIN_ROOT")) {
      ac6_toolchain_path = RteUtils::RemoveQuotes(line);
      break;
    }
  }
}

void CBuildIntegTestEnv::TearDown() {
  RunScript("tear_down.sh", testout_folder);
}

int main(int argc, char **argv) {
  try {
    testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new CBuildIntegTestEnv);
    return RUN_ALL_TESTS();
  }
  catch (testing::internal::GoogleTestFailureException const& e) {
    std::cout << "runtime_error: " << e.what();
    return 2;
  }
  catch (...) {
    std::cout << "non-standard exception";
    return 2;
  }
}
