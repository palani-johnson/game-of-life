import matplotlib.pyplot as plt
import subprocess

serial = {
    64: 0.167311,
    128: 0.622848,
    256: 2.376215,
    512: 9.526162,
    1024: 38.431477,
    2048: 154.439860,
}

omp = {
    (1, 64): 0.033329,
    (1, 128): 0.069598,
    (1, 256): 0.253520,
    (1, 512): 1.006112,
    (1, 1024): 4.016103,
    (1, 2048): 16.072131,
    (2, 64): 0.009560,
    (2, 128): 0.037351,
    (2, 256): 0.131559,
    (2, 512): 0.514813,
    (2, 1024): 2.052374,
    (2, 2048): 8.198974,
    (4, 64): 0.006210,
    (4, 128): 0.020486,
    (4, 256): 0.079142,
    (4, 512): 0.272811,
    (4, 1024): 1.073166,
    (4, 2048): 4.271183,
    (8, 64): 0.005289,
    (8, 128): 0.015901,
    (8, 256): 0.062283,
    (8, 512): 0.225722,
    (8, 1024): 0.653508,
    (8, 2048): 2.362771,
    (16, 64): 0.038929,
    (16, 128): 0.043004,
    (16, 256): 0.089383,
    (16, 512): 0.217259,
    (16, 1024): 0.568462,
    (16, 2048): 2.155147,
}

cuda = {
    64: 1.780905,
    128: 0.180122,
    256: 0.284672,
    512: 0.436926,
    1024: 1.241556,
    2048: 4.985203,
}

mpi = {
    (2, 64): 0.212493,
    (2, 128): 0.689728,
    (2, 256): 2.382074,
    (2, 512): 9.144672,
    (2, 1024): 36.930383,
    (2, 2048): 145.165876,
    (4, 64): 0.193594,
    (4, 128): 0.562609,
    (4, 256): 2.364476,
    (4, 512): 7.685617,
    (4, 1024): 30.547007,
    (4, 2048): 121.286129,
    (8, 64): 0.210083,
    (8, 128): 0.662010,
    (8, 256): 2.437906,
    (8, 512): 9.156283,
    (8, 1024): 36.183158,
    (8, 2048): 140.689063,
}

serial_no_io = {
    64: 0.203340,
    128: 0.585172,
    256: 2.315953,
    512: 9.340336,
    1024: 37.614411,
    2048: 151.044947,
}

omp_no_io = {
    (1, 64): 0.014796,
    (1, 128): 0.064648,
    (1, 256): 0.247866,
    (1, 512): 0.983896,
    (1, 1024): 3.951315,
    (1, 2048): 15.869686,
    (2, 64): 0.007927,
    (2, 128): 0.032807,
    (2, 256): 0.125504,
    (2, 512): 0.496209,
    (2, 1024): 1.984131,
    (2, 2048): 7.952195,
    (4, 64): 0.004198,
    (4, 128): 0.016935,
    (4, 256): 0.066087,
    (4, 512): 0.253091,
    (4, 1024): 1.025693,
    (4, 2048): 4.001923,
    (8, 64): 0.002465,
    (8, 128): 0.009002,
    (8, 256): 0.038520,
    (8, 512): 0.142573,
    (8, 1024): 0.574375,
    (8, 2048): 2.084302,
    (16, 64): 0.002746,
    (16, 128): 0.007548,
    (16, 256): 0.037307,
    (16, 512): 0.121797,
    (16, 1024): 0.440369,
    (16, 2048): 1.860628,
}

cuda_no_io = {
    64: 1.759342,
    128: 0.126090,
    256: 0.161286,
    512: 0.122419,
    1024: 0.267133,
    2048: 0.545301,
}

mpi_no_io = {
    (1, 64): 0.196043,
    (1, 128): 0.608096,
    (1, 256): 1.958814,
    (1, 512): 7.509606,
    (1, 1024): 29.837960,
    (1, 2048): 119.927398,
    (2, 64): 0.143212,
    (2, 128): 0.391217,
    (2, 256): 1.097359,
    (2, 512): 4.017873,
    (2, 1024): 15.945836,
    (2, 2048): 62.966239,
    (4, 64): 0.131518,
    (4, 128): 0.346854,
    (4, 256): 0.685989,
    (4, 512): 2.216044,
    (4, 1024): 8.554306,
    (4, 2048): 34.285132,
    (8, 64): 0.147848,
    (8, 128): 0.237900,
    (8, 256): 0.519152,
    (8, 512): 1.495542,
    (8, 1024): 5.538495,
    (8, 2048): 22.122593,
}


