from setuptools import setup

def readme():
    with open('README.rst') as f:
        return f.read()

setup(name='code',
      version='0.1',
      description='ZOYI_KOLON_Analysis',
      long_description='ZOYI_KOLON_Analysis code for upcoming publications',
      classifiers=[ ### https://pypi.python.org/pypi?%3Aaction=list_classifiers
        'Development Status :: 1 - Planning',
        'License :: OSI Approved :: Python Software Foundation License',
        'Natural Language :: Korean',
        'Programming Language :: Python :: 3',
        'Topic :: Database',
      ],
      keywords='funniest joke comedy flying circus',
      url='https://github.com/Seondong/MSRA_proposal_wifi_log',
      author='Sundong Kim',
      author_email='sundong.kim@kaist.ac.kr',
      license='MIT',
      scripts=['bin/code-command'],
 	  entry_points={
 	  	'console_scripts': ['code_command=code.command_line:main'],
 	  },
      test_suite='nose.collector',
      tests_require=['nose'],
      packages=['code'],
      install_requires=[
      		'markdown',
      ],
      zip_safe=False)