#!/usr/bin/env python

# Copyright (c) 2015 Google Inc. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Make sure msvs_target_platform_version works correctly.
"""

import TestGyp

import os
import sys
import struct

CHDIR = 'winuwp-target-platform-version'

print 'This test is not currently working on the bots: https://code.google.com/p/gyp/issues/detail?id=466'
sys.exit(0)

if (sys.platform == 'win32' and
    int(os.environ.get('GYP_MSVS_VERSION', 0)) == 2015):
  test = TestGyp.TestGyp(formats=['msvs'])

  test.run_gyp('winuwp-target-platform-version.gyp', chdir=CHDIR)

  test.build('winuwp-target-platform-version.gyp',
             'enable_winuwp_10_platversion_dll', chdir=CHDIR)

  # Target Platform without Minimum Target Platform version defaults to a valid
  # Target Platform and compiles.
  test.build('winuwp-target-platform-version.gyp',
             'enable_winuwp_10_platversion_nominver_dll', chdir=CHDIR)

  # Target Platform is set to 9.0 which is invalid for 2015 projects so
  # compilation must fail.
  test.build('winuwp-target-platform-version.gyp',
             'enable_winuwp_9_platversion_dll', chdir=CHDIR, status=1)

  # Missing Target Platform for 2015 projects must fail.
  test.build('winuwp-target-platform-version.gyp',
             'enable_winuwp_missing_platversion_dll', chdir=CHDIR, status=1)

  test.pass_test()
