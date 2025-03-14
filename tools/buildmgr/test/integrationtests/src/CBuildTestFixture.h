/*
 * Copyright (c) 2020-2021 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "CBuildIntegTestEnv.h"

class CBuildTestFixture :public ::testing::Test {
protected:
  void RunCBuildScript          (const TestParam& param);
  void RunCBuildScriptClean     (const TestParam& param);
  void RunCBuildScriptWithArgs  (const TestParam& param);
  void CheckCMakeLists          (const TestParam& param);
  void CheckMapFile             (const TestParam& param);
  void CheckOutputDir           (const TestParam& param, const std::string& outdir);
  void CheckCMakeIntermediateDir(const TestParam& param, const std::string& intdir);
  void CleanOutputDir           (const TestParam& param);
};
