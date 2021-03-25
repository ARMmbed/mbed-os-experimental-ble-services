import pytest
from data_gen import DataGenerator

@pytest.fixture
def data_gen():
    return data_gen(128)

def test_chunk_length(d):
    a = d.get_chunk_n(0)
    assert len(a) == d.get_chunk_size()

def test_lazy_generation(d):
    assert len(d.chunks) == 0
    a = d.get_chunk_n(1)
    assert len(d.chunks) == 2

def test_persistence(d):
    a = d.get_chunk_n(10)
    b = d.get_chunk_n(2)
    c = d.get_chunk_n(10)
    assert a == c
