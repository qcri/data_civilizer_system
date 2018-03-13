from setuptools import setup, Extension
setup(name='goldenrecord',
      version='0.1',
      ext_modules=[Extension('_goldenrecord', sources=['Aggregator.cc', 'CSVReader.cc', 'Reader.cc', 'Rules.cc', 'Synthesize.cc', 'Table.cc', 'Wrapper.cc', 'Consolidation.cc', 'goldenrecord.i'],
        swig_opts=['-c++', '-py3'], extra_compile_args=['-O3', '-std=c++14', '-DBOTH_AGGREGATION_ENABLE', '-DLOCAL_THRESHOLD_ENABLE', '-DGLOBAL_THRESHOLD_ENABLE', '-DUNIQUE_THRESHOLD_ENABLE', '-DSTATIC_ORDERING_ENABLE', '-DSINGLE_CONSTANT_TERM_ENABLE', '-DPREFIX_SUFFIX_TERM_ENABLE'])],
      headers=['Aggregator.h', 'CSVReader.h', 'Reader.h', 'Rules.h', 'Synthesize.h', 'Table.h', 'Wrapper.h', 'Consolidation.h']
)
