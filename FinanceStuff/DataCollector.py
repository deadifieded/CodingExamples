import numpy
import math
import requests
import matplotlib.pyplot as plt
import time
import calendar
from Utils import writeUniormDataBin, readUniformDataBin

headers = {
	"accept": "application/json",
	"APCA-API-KEY-ID": "PKJI8IU1OQXIOY2Y7Z8T",
	"APCA-API-SECRET-KEY": "qt3rkI6zEq9xPOSZcpXX6asu7zcBJJLSfcJLSh89"
}

ALL_SYMBOLS_PATH = "Data/AllSymbols.txt"
PROGRESS_PATH = "Data/Progress.txt"
GATHERED_SYMBOLS_PATH = "Data/GatheredSymbols.txt"
PRICE_DATA_PATH = "Data/PriceData/"
DEFAULT_START = "1990-01-01T11:10:00Z"

class Timer:
	
	def __init__(self, delta):
		self.last = time.time()
		self.delta = delta

	def pause(self):
		now = time.time()
		time.sleep(max(0,self.delta-now+self.last))
		self.last = now


request_timer = Timer(60/197)


def replaceColons(string):
	new_string = ""
	for i in string:
		if i == ":":
			new_string+="%3A"
		else:
			new_string+=i
	return new_string


def getAllSymbols():

	url = "https://paper-api.alpaca.markets/v2/assets?attributes="

	all_symbols = []

	request_timer.pause()

	response = requests.get(url, headers=headers)

	for i in response.json():
		if i["class"]=="us_equity" and i["tradable"] == True:
			all_symbols.append(i["symbol"])

	return all_symbols


def getBars(symbol, start, interval):
	url = "https://data.alpaca.markets/v2/stocks/bars?timeframe=" + interval + "&symbols="+symbol+"&start="+replaceColons(start)+"&limit=10000&feed=iex"

	while True:
		try:
			request_timer.pause()
			response = requests.get(url, headers=headers)
			break
		except:
			print("oops")

	priceData = []
	
	try:
		return response.json()["bars"][symbol]
	except:
		print(response.text)
		return []



TIME_FORMAT_STRING = "%Y-%m-%dT%H:%M:%SZ"


def getUTC(time_string):
	return calendar.timegm(time.strptime(time_string, TIME_FORMAT_STRING))


def getTimeString(utc):
	return time.strftime(TIME_FORMAT_STRING, time.gmtime(utc))


def getPercentBelowOrEqual(lst, threshold):
	total = 0
	less_or_equal = 0

	for i in lst:
		if i <= threshold:
			less_or_equal +=1
		total+=1

	if total == 0:
		return 0

	return less_or_equal/total






all_symbols = []
progress = 0

number_passed = 0

try:
	with open(ALL_SYMBOLS_PATH, "r") as file:
		for line in file:
			all_symbols.append(line.strip())
		
		if len(all_symbols)<1000:
			raise("your own damn kids")

	with open(PROGRESS_PATH, "r") as file:
		for line in file:
			progress = int(line.strip())

	with open(GATHERED_SYMBOLS_PATH, "r") as file:
		for i in file:
			number_passed+=1
except:
	with open(ALL_SYMBOLS_PATH, "w") as file:
		all_symbols = getAllSymbols()
		for i in all_symbols:
			file.write(i+"\n")
	
	with open(PROGRESS_PATH, "w") as file:
		progress = 0
		file.write("0\n")

	with open(GATHERED_SYMBOLS_PATH, "w") as file:
		pass


print(len(all_symbols))

for symbol_index in range(progress,len(all_symbols)):
	
	symbol = all_symbols[symbol_index]

	start = DEFAULT_START

	with open(PROGRESS_PATH, "a") as file:
		file.write(str(symbol_index)+"\n")

	last = -1
	earliest = 0

	price_data = []
	time_data = []

	while True:

		bars = getBars(symbol, start, "1Min")

		nof_bars = len(bars)

		if nof_bars<1000:
			break


		if last < 0:
			gaps = []
			earliest = getUTC(bars[0]["t"])

		else:
			gaps = [getUTC(bars[0]["t"])-last]

		last_bar_time = getUTC(bars[0]["t"])

		bar_times = [last_bar_time]

		for i in range(1,nof_bars):
			temp = getUTC(bars[i]["t"])
			bar_times.append(temp)
			gaps.append(temp-last_bar_time)
			last_bar_time = temp

		max_gap = max(gaps)

		if max_gap/60/60/24 >= 4:
			break

		percent_adjacent = getPercentBelowOrEqual(gaps,60)

		if percent_adjacent < 0.5:
			break



		first_unique_bar = 0

		for i in range(nof_bars):
			if bar_times[i]>last:
				first_unique_bar = i
				break

		last = getUTC(bars[-1]["t"])

		start = bars[-1]["t"]

		#print(symbol+" "+getTimeString(last))

		for i in range(first_unique_bar, nof_bars):
			_open = bars[i]["o"]
			_high = bars[i]["h"]
			_low = bars[i]["l"]
			_close = bars[i]["c"]

			_time = bar_times[i]

			if _open<=0 or _high<=0 or _low <= 0 or _close <=0:
				continue

			price_data.append(_open)
			time_data.append(_time)

			if _open>_close:
				price_data.append(_high)
				price_data.append(_low)
			else:
				price_data.append(_low)
				price_data.append(_high)

			time_data.append(_time)
			time_data.append(_time)

			price_data.append(_close)
			time_data.append(_time)

	if last>=0:

		nof_data_points = min(len(price_data),len(time_data))

		print(nof_data_points)

		with open(PRICE_DATA_PATH+symbol+".bin", "wb") as file:
			writeUniformDataBin(file, "[q][d]", [time_data, price_data])

		with open(GATHERED_SYMBOLS_PATH, "a") as file:
			file.write(symbol+"\n")
		
		number_passed+=1
		print()
		print(number_passed)
		print(str(symbol_index) + " " + symbol)
		print("From " + getTimeString(earliest) + " till " + getTimeString(last))
