Run `python3 setup.py` to install. 

`LZS.compress(data : bytes, optimal : bool = False) -> bytes` and `LZS.decompress(data : bytes, expected_size : Optional[int]) -> bytes` are the two exported functionalities.

`compress` will compress `data` using a hash-chain implementation. If optimal compression is desired, at cost of all speed, then set `optimal`.

`decompress` will decompress `data`, allocating to `expected_size` if provided. Yes, this functionality is insecure. This will be an item to fix if it's worth putting this on PIP. It is currently not.