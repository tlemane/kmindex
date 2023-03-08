from typing import List, Union, Iterator

class Query:
    PAYLOAD = {
        "id": "",
        "seq": [""],
        "index": [""],
        "r": 0.0,
        "z": 3
    }

    def __init__(self,
                 qid: str,
                 sequence: Union[str, List[str]],
                 index: Union[str, List[str]],
                 r: float=0.0,
                 z: int=3) -> None:
        self.qid = qid
        self.sequences = self._wrap_as_list(sequence)
        self.indexes = self._wrap_as_list(index)
        self.z = 3
        self.r = 0.0


    def _wrap_as_list(self, v: Union[str, List[str]]) -> List[str]:
        if isinstance(v, str):
            return [v]
        return v

    @property
    def payload(self):
        return {
            "id": self.qid,
            "seq": self.sequences,
            "index": self.indexes,
            "r": self.r,
            "z": self.z
        }

class QBatch:
    def __init__(self) -> None:
        self.queries: List[Query] = []

    def push(self,
             qid: str,
             sequence: Union[str, List[str]],
             index: Union[str, List[str]],
             r: float=0.0,
             z: int=3) -> None:
        self.queries.append(Query(qid, sequence, index, r, z))

    def __iter__(self) -> Iterator[Query]:
        return iter(self.queries)

class QResponse:
    def __init__(self, data) -> None:
        self.index = {}
        self._from_json(data)

    def _from_json(self, data) -> None:
        self.id = list(data[list(data.keys())[0]].keys())[0]
        for k in data.keys():
            self.result[k] = data[k][self.id]

