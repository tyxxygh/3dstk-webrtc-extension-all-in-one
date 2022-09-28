# Copyright (c) 2013 Google Inc. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
 'targets': [
    {
      'target_name': 'enable_winuwp_81_revision_dll',
      'type': 'shared_library',
      'msvs_enable_winuwp': 1,
      'msvs_application_type_revision': '8.1'
      'sources': [
        'dllmain.cc',
      ],
    },
    {
      'target_name': 'enable_winuwp_82_revision_dll',
      'type': 'shared_library',
      'msvs_enable_winuwp': 1,
      'msvs_application_type_revision': '8.2'
      'sources': [
        'dllmain.cc',
      ],
    },
    {
      'target_name': 'enable_winuwp_invalid_revision_dll',
      'type': 'shared_library',
      'msvs_enable_winuwp': 1,
      'msvs_application_type_revision': '999'
      'sources': [
        'dllmain.cc',
      ],
    },
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
