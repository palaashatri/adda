import json
import urllib.request
import os

json_data = ''

if __name__ == '__main__' :
    os.mkdir('./images')
    with open('./data.json', 'r') as f : 
        json_data = json.loads(f.read())

    data = json_data['data']

    for data_field in data:
        os.mkdir('./images/' + data_field)
        sizes = data[data_field].keys()
        for size in sizes :
            os.mkdir('./images/' + data_field + '/' + size )
            url = data[data_field][size]
            print('downloading : ', url)
            urllib.request.urlretrieve(url, './images/' + data_field + '/' + size + f'/image_{data_field}_{size}.jpg')
