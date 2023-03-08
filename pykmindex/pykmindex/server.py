import uplink
import asyncio

from uplink import Consumer, Body, post, get, json
from pykmindex.query import Query, QBatch, QResponse

def connect(url: str, port: int):
    return KmIndexServer(url, port)

class KmIndexAPI(Consumer):

    @get("kmindex/infos")
    def infos(self):
        "Fetches indexes informations."

    @json
    @post("kmindex/query")
    def query(self, data: Body):
        "Query index."


async def query(async_client, payload):
    response = await async_client.query()

class KmIndexServer:
    def __init__(self, url: str, port: int):
        self._url = url
        self._port = port
        self._api = KmIndexAPI(base_url = f"{self._url}:{self._port}")
        self._api_async = KmIndexAPI(base_url = f"{self._url}:{self._port}", client = uplink.AiohttpClient())
        self.infos = dict(self._api.infos().json())

    def submit(self, query: Query):
        return QResponse(self._api.query(query.payload).json())

    async def async_query(self, query: Query):
        response = await self._api_async.query(query.payload)
        json = await response.json(content_type=None)
        return QResponse(json)

    def submit_async(self, queries: QBatch):
        futures = [self.async_query(q) for q in queries]
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        return loop.run_until_complete(asyncio.gather(*futures))


