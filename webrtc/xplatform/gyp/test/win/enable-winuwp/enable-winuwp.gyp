# Copyright (c) 2013 Google Inc. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
 'targets': [
    {
      'target_name': 'enable_winuwp_dll',
      'type': 'shared_library',
      'msvs_enable_winuwp': 1,
      'sources': [
        'dllmain.cc',
      ],
    },
    {
      'target_name': 'enable_winuwp_missing_dll',
      'type': 'shared_library',
      'sources': [
        'dllmain.cc',
      ],
    },
    {
      'target_name': 'enable_winuwp_winphone_dll',
      'type': 'shared_library',
      'msvs_enable_winuwp': 1,
      'msvs_enable_winphone': 1,
      'sources': [
        'dllmain.cc',
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'AdditionalDependencies': [
            '%(AdditionalDependencies)',
          ],
        },
      },
    },
  ]
}
