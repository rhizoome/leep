from leep.extension import get as get_c
from leep.extension import set as set_c
from timeit import timeit
from collections.abc import Sequence

_c = {4: 42}
_b = [_c]
_a = {"key": _b}
_p = "key.0.4"


def call_set_c():
    set_c(_a, _p, 3)
    assert get_c(_a, _p) == 3


def call_get_c():
    assert get_c(_a, _p) == 3


def call_get_py():
    assert get_py(_a, _p) == 3


def get_py(obj, path, delim="."):
    result = obj
    for part in path.split(delim):
        if isinstance(result, Sequence):
            result = result[int(part)]
        else:
            try:
                result = result[part]
            except KeyError:
                result = result[int(part)]
    return result


def test_leep():
    # call_set_c()
    # call_get_c()
    print(timeit(call_set_c))
    print(timeit(call_get_c))
    print(timeit(call_get_py))
