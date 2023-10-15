from leep.extension import path_expr as path_expr_c
from timeit import timeit
from collections.abc import Sequence

_c = {4: 42}
_b = [_c]
_a = {"key": _b}
_p = "key.0.4"


def access_c():
    assert path_expr_c(_a, _p.replace(".", "\0")) == 42


def access_py():
    assert path_expr_py(_a, _p.replace(".", "\0")) == 42


def path_expr_py(obj, path):
    result = obj
    for part in path.split("\0"):
        if isinstance(result, Sequence):
            result = result[int(part)]
        else:
            try:
                result = result[part]
            except KeyError:
                result = result[int(part)]
    return result


def test_leep():
    print(timeit(access_c))
    print(timeit(access_py))
