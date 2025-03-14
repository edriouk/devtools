/*
 * Copyright (c) 2020-2021 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "CBuildGenTestFixture.h"

using namespace std;

void CBuildGenTestFixture::CheckCMakeLists(const TestParam& param) {
  int ret_val;
  ifstream f1, f2;
  string l1, l2;

  string filename1 = testout_folder + "/" + param.name + "/IntDir/CMakeLists.txt";
  f1.open(filename1);
  ret_val = f1.is_open();
  ASSERT_EQ(ret_val, true) << "Failed to open " << filename1;

  string filename2 = testout_folder + "/" + param.name + "/CMakeLists.txt.ref";
  f2.open(filename2);
  ret_val = f2.is_open();
  ASSERT_EQ(ret_val, true) << "Failed to open " << filename2;

  while (getline(f1, l1) && getline(f2, l2)) {
    if (!l1.empty() && l1.rfind('\r') == l1.length() - 1) {
      l1.pop_back(); // remove eol cr
    }

    if (!l2.empty() && l2.rfind('\r') == l2.length() - 1) {
      l2.pop_back();
    }

    if (l1 != l2) {
      // ignore commented lines
      if ((!l1.empty() && l1.at(0) == '#') && (!l2.empty() && l2.at(0) == '#')) {
        continue;
      }

      // ignore lines containing forward slashes (paths)
      if ((!l1.empty() && (l1.find("/") != string::npos)) && (!l2.empty() && (l2.find("/") != string::npos))) {
        continue;
      }

      FAIL() << filename1 << " is different from " << filename2;
    }
  }

  f1.close();
  f2.close();
}

void CBuildGenTestFixture::RunCBuildGen(const TestParam& param, string projpath, bool env) {
  string cmd, workingDir;
  error_code ec;

  if (projpath != examples_folder)
    workingDir = projpath + "/" + param.name;
  else {
    workingDir = testout_folder + "/" + param.name;

    if (fs::exists(workingDir, ec)) {
      RemoveDir(workingDir);
    }

    fs::create_directories(workingDir, ec);
    fs::copy(fs::path(examples_folder + "/" + param.name), fs::path(workingDir),
      fs::copy_options::recursive, ec);
  }

  if (!env) {
    cmd = testout_folder + "/cbuild/bin/cbuildgen \"" + workingDir +
      '/' + param.targetArg + ".cprj\" " + param.command +
      (param.options.empty() ? "" : " ") + param.options;
  }
  else {
    cmd = "bash -c \"source " + testout_folder +
      "/cbuild/etc/setup && cbuildgen \"" + workingDir +
      '/' + param.targetArg + ".cprj\" " + param.command +
      (param.options.empty() ? "" : " ") + param.options + "\"";
  }
  auto ret_val = system(cmd.c_str());
  ASSERT_EQ(param.expect, (ret_val == 0) ? true : false);
}

void CBuildGenTestFixture::RunCBuildGenAux(const string& cmd, bool expect) {
  string command = "bash -c \"source " + testout_folder +
    "/cbuild/etc/setup && cbuildgen " + cmd + "\"";
  int ret_val = system(command.c_str());
  ASSERT_EQ(expect, (ret_val == 0) ? true : false);
}

void CBuildGenTestFixture::RunLayerCommand(int cmdNum, const TestParam& param) {
  int ret_val;
  string cmdName, layers, output;
  error_code ec;

  string workingDir = testout_folder + "/" + param.name;
  if (fs::exists(workingDir, ec)) {
    RemoveDir(workingDir);
  }

  fs::create_directories(workingDir, ec);
  fs::copy(fs::path(examples_folder + "/" + param.name), fs::path(workingDir),
    fs::copy_options::recursive, ec);

  // clean test case working directory
  workingDir += "/Project";
  if (fs::exists(workingDir, ec) && fs::is_directory(workingDir, ec)) {
    for (auto& p : fs::recursive_directory_iterator(workingDir, ec)) {
      if (fs::is_regular_file(p, ec)) fs::remove(p, ec);
    }
  }
  else {
    fs::create_directories(workingDir, ec);
  }

  // set parameters and copy files
  switch (cmdNum) {
  case 1:
    cmdName = "extract";
    fs::copy(fs::path(workingDir + "/../Project_Full"), fs::path(workingDir), fs::copy_options::recursive, ec);
    output = " --outdir=" + workingDir + "/Layer";
    break;
  case 2:
    cmdName = "compose";
    layers = workingDir + "/../Layer_Ref/application/application.clayer " + workingDir + "/../Layer_Ref/device/device.clayer";
    break;
  case 3:
    cmdName = "add";
    layers = workingDir + "/../Layer_Ref/device/device.clayer";
    fs::copy(fs::path(workingDir + "/../Project_Partial"), fs::path(workingDir), fs::copy_options::recursive, ec);
    break;
  case 4:
    cmdName = "remove";
    layers = "--layer=device";
    fs::copy(fs::path(workingDir + "/../Project_Full"), fs::path(workingDir), fs::copy_options::recursive, ec);
    break;
  }

  string cmd = "bash -c \"source " + testout_folder + "/cbuild/etc/setup && cbuildgen " + workingDir + "/" + param.targetArg + ".cprj " +
    cmdName + " " + layers + output + "\"";
  ret_val = system(cmd.c_str());
  ASSERT_EQ(ret_val, 0);
}

void CBuildGenTestFixture::CheckDescriptionFiles(const string& filename1, const string& filename2) {
  int ret_val;
  ifstream f1, f2;
  string l1, l2;

  f1.open(filename1);
  ret_val = f1.is_open();
  ASSERT_EQ(ret_val, true) << "Failed to open " << filename1;

  f2.open(filename2);
  ret_val = f2.is_open();
  ASSERT_EQ(ret_val, true) << "Failed to open " << filename2;

  while (getline(f1, l1) && getline(f2, l2)) {
    // remove eol cr
    if (!l1.empty() && l1.rfind('\r') == l1.length() - 1) {
      l1.pop_back();
    }

    if (!l2.empty() && l2.rfind('\r') == l2.length() - 1) {
      l2.pop_back();
    }

    if (l1 != l2) {
      // ignore 'timestamp'
      if ((!l1.empty() && (l1.find("timestamp=") != string::npos)) && (!l2.empty() && (l2.find("timestamp=") != string::npos))) {
        continue;
      }

      // ignore 'used file'
      if ((!l1.empty() && (l1.find("used file=") != string::npos)) && (!l2.empty() && (l2.find("used file=") != string::npos))) {
        continue;
      }

      FAIL() << filename1 << " is different from " << filename2;
    }
  }

  f1.close();
  f2.close();
}


void CBuildGenTestFixture::GetDirectoryItems(const string& inPath, set<string> &Result, const string& ignoreDir) {
  string itemPath;
  for (auto& item : fs::directory_iterator(inPath)) {
    if (fs::is_directory(fs::status(item))) {
      if (item.path().filename().compare(ignoreDir) == 0) {
        continue;
      }
      GetDirectoryItems(item.path().generic_string(), Result, ignoreDir);
    }
    itemPath = item.path().generic_string();
    Result.insert(itemPath.substr(inPath.length() + 1, itemPath.length()));
  }
}

void CBuildGenTestFixture::CheckLayerFiles(const TestParam& param) {
  string layerDir = testout_folder + "/" + param.name + "/Project/Layer";
  string layerRef = testout_folder + "/" + param.name + "/Layer_Ref";
  error_code ec;
  set<string> dir, ref, clayers, clayers_ref;

  GetDirectoryItems(layerDir, dir, "RTE");
  GetDirectoryItems(layerRef, ref, "RTE");

  for (auto& p : fs::recursive_directory_iterator(layerDir, ec)) {
    if (p.path().extension() == ".clayer") {
      clayers.insert(p.path().generic_string());
    }
  }

  for (auto& p : fs::recursive_directory_iterator(layerRef, ec)) {
    if (p.path().extension() == ".clayer") {
      clayers_ref.insert(p.path().generic_string());
    }
  }

  ASSERT_EQ(dir == ref, true) << "Layer directory '" << layerDir
    << "' filetree is different from '" << layerRef << "' reference";

  auto clayer = clayers.begin();
  auto clayer_ref = clayers_ref.begin();

  for (; clayer != clayers.end() && clayer_ref != clayers_ref.end(); clayer++, clayer_ref++) {
    CheckDescriptionFiles(*clayer, *clayer_ref);
  }
}

void CBuildGenTestFixture::CheckProjectFiles(const TestParam& param) {
  string projectDir = testout_folder + "/" + param.name + "/Project";
  string projectRef = testout_folder + "/" + param.name + "/Project_Ref";
  error_code ec;
  set<string> dir, ref;

  GetDirectoryItems(projectDir, dir, "RTE");
  GetDirectoryItems(projectRef, ref, "RTE");

  ASSERT_EQ(dir == ref, true) << "Project directory '" << projectDir
    << "' filetree is different from '" << projectRef << "' reference";

  CheckDescriptionFiles(projectDir + "/" + param.targetArg + ".cprj", projectRef + "/" + param.targetArg + ".cprj");
}

void CBuildGenTestFixture::CheckOutputDir(const TestParam& param, const string& outdir) {
  error_code ec;
  EXPECT_EQ(param.expect, fs::exists(outdir + "/" + param.targetArg + ".clog", ec))
    << "File " << param.targetArg << ".clog does " << (param.expect ? "not " : "") << "exist!";
}

void CBuildGenTestFixture::CheckCMakeIntermediateDir(const TestParam& param, const string& intdir) {
  error_code ec;
  EXPECT_EQ(param.expect, fs::exists(intdir + "/CMakeLists.txt", ec))
    << "File CMakeLists.txt does " << (param.expect ? "not " : "") << "exist!";
}

void CBuildGenTestFixture::CheckCPInstallFile(const TestParam& param) {
  int ret_val;
  ifstream f1, f2;
  string l1, l2;

  string filename1 = testdata_folder + "/" + param.name + "/" + param.targetArg + ".cpinstall";
  f1.open(filename1);
  ret_val = f1.is_open();
  ASSERT_EQ(ret_val, true) << "Failed to open " << filename1;

  string filename2 = testdata_folder + "/" + param.name + "/" + param.targetArg + ".cpinstall.ref";
  f2.open(filename2);
  ret_val = f2.is_open();
  ASSERT_EQ(ret_val, true) << "Failed to open " << filename2;

  while (getline(f1, l1) && getline(f2, l2)) {
    if (!l1.empty() && l1.rfind('\r') == l1.length() - 1) {
      l1.pop_back(); // remove eol cr
    }

    if (!l2.empty() && l2.rfind('\r') == l2.length() - 1) {
      l2.pop_back();
    }

    size_t s;
    if ((s = l1.find("https", 0)) != string::npos) {
      l1.replace(s, 5, "http"); // replace 'https' by 'http'
    }

    if ((s = l2.find("https", 0)) != string::npos) {
      l2.replace(s, 5, "http");
    }

    if (l1 != l2) {
      FAIL() << filename1 << " is different from " << filename2;
    }
  }

  f1.close();
  f2.close();
}
