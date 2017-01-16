### How to run this simple test
### $ pip install nose
### $ nosetests

from unittest import TestCase

import code

class TestJoke(TestCase):
    def test_is_string(self):
        s = code.joke()
        self.assertTrue(isinstance(s, basestring))