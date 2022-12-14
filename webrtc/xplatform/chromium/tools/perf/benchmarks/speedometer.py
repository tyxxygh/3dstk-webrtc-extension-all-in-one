# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Apple's Speedometer performance benchmark.

Speedometer measures simulated user interactions in web applications.

The current benchmark uses TodoMVC to simulate user actions for adding,
completing, and removing to-do items. Speedometer repeats the same actions using
DOM APIs - a core set of web platform APIs used extensively in web applications-
as well as six popular JavaScript frameworks: Ember.js, Backbone.js, jQuery,
AngularJS, React, and Flight. Many of these frameworks are used on the most
popular websites in the world, such as Facebook and Twitter. The performance of
these types of operations depends on the speed of the DOM APIs, the JavaScript
engine, CSS style resolution, layout, and other technologies.
"""

import os

from core import path_util
from core import perf_benchmark

from telemetry import benchmark
from telemetry import page as page_module
from telemetry.page import legacy_page_test
from telemetry import story
from telemetry.value import list_of_scalar_values

from metrics import keychain_metric


_SPEEDOMETER_DIR = os.path.join(path_util.GetChromiumSrcDir(),
    'third_party', 'WebKit', 'PerformanceTests', 'Speedometer')


class SpeedometerMeasurement(legacy_page_test.LegacyPageTest):
  enabled_suites = [
      'VanillaJS-TodoMVC',
      'EmberJS-TodoMVC',
      'BackboneJS-TodoMVC',
      'jQuery-TodoMVC',
      'AngularJS-TodoMVC',
      'React-TodoMVC',
      'Flight-TodoMVC'
  ]

  def __init__(self):
    super(SpeedometerMeasurement, self).__init__()

  def CustomizeBrowserOptions(self, options):
    keychain_metric.KeychainMetric.CustomizeBrowserOptions(options)

  def ValidateAndMeasurePage(self, page, tab, results):
    tab.WaitForDocumentReadyStateToBeComplete()
    iterationCount = 10
    # A single iteration on android takes ~75 seconds, the benchmark times out
    # when running for 10 iterations.
    if tab.browser.platform.GetOSName() == 'android':
      iterationCount = 3

    tab.ExecuteJavaScript("""
        // Store all the results in the benchmarkClient
        benchmarkClient._measuredValues = []
        benchmarkClient.didRunSuites = function(measuredValues) {
          benchmarkClient._measuredValues.push(measuredValues);
          benchmarkClient._timeValues.push(measuredValues.total);
        };
        benchmarkClient.iterationCount = {{ count }};
        startTest();
        """,
        count=iterationCount)
    tab.WaitForJavaScriptCondition(
        'benchmarkClient._finishedTestCount == benchmarkClient.testsCount',
        timeout=600)
    results.AddValue(list_of_scalar_values.ListOfScalarValues(
        page, 'Total', 'ms',
        tab.EvaluateJavaScript('benchmarkClient._timeValues'),
        important=True))

    # Extract the timings for each suite
    for suite_name in self.enabled_suites:
      results.AddValue(list_of_scalar_values.ListOfScalarValues(
          page, suite_name, 'ms',
          tab.EvaluateJavaScript("""
              var suite_times = [];
              for(var i = 0; i < benchmarkClient.iterationCount; i++) {
                suite_times.push(
                    benchmarkClient._measuredValues[i].tests[{{ key }}].total);
              };
              suite_times;
              """,
              key=suite_name), important=False))
    keychain_metric.KeychainMetric().AddResults(tab, results)


@benchmark.Owner(emails=['bmeurer@chromium.org', 'mvstanton@chromium.org'])
class Speedometer(perf_benchmark.PerfBenchmark):
  test = SpeedometerMeasurement

  @classmethod
  def Name(cls):
    return 'speedometer'

  def CreateStorySet(self, options):
    ps = story.StorySet(base_dir=_SPEEDOMETER_DIR,
        serving_dirs=[_SPEEDOMETER_DIR])
    ps.AddStory(page_module.Page(
        'file://index.html', ps, ps.base_dir, name='Speedometer'))
    return ps

  def GetExpectations(self):
    class StoryExpectations(story.expectations.StoryExpectations):
      def SetExpectations(self):
        pass # Speedometer1.0 not disabled.
    return StoryExpectations()
