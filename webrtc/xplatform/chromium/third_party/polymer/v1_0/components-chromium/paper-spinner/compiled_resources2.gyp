# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# NOTE: Created with generate_compiled_resources_gyp.py, please do not edit.
{
  'targets': [
    {
      'target_name': 'paper-spinner-behavior-extracted',
      'includes': ['../../../../closure_compiler/compile_js2.gypi'],
    },
    {
      'target_name': 'paper-spinner-extracted',
      'dependencies': [
        'paper-spinner-behavior-extracted',
      ],
      'includes': ['../../../../closure_compiler/compile_js2.gypi'],
    },
    {
      'target_name': 'paper-spinner-lite-extracted',
      'dependencies': [
        'paper-spinner-behavior-extracted',
      ],
      'includes': ['../../../../closure_compiler/compile_js2.gypi'],
    },
  ],
}
