# Copyright (c) 2016 Google Inc. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import TestGyp

import os
import sys

if (sys.platform == 'win32' and
    int(os.environ.get('GYP_MSVS_VERSION', 0)) >= 2015):
  test = TestGyp.TestGyp(formats=['msvs'])

  CHDIR = 'compiler-flags'

  test.run_gyp('compile-as-winuwp.gyp', chdir=CHDIR)

  test.build('compile-as-winuwp.gyp', 'test-compile-as-winuwp', chdir=CHDIR)

  test.pass_test()