project('truth-table-generator', 'cpp', default_options: ['cpp_std=c++20'])

gtk_dep = dependency('gtkmm-4.0', version: '>=4.0')

executable('truth-table-generator',
           'main.cpp',
           dependencies: [gtk_dep],
           install: true)