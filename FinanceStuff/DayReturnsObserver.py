import os
import numpy
import math
import struct
from Utils import readUniformDataBin, writeUniformDataBin
import time

#region

class LiquidityEngine:
	m = 1.05
	l = 2
	q=numpy.sqrt(l)/(numpy.sqrt(l)-1)
	p=1
	a=0.5
	b=0.5
	a_i = 0
	b_i = 0
	mp=1
	lp=1

	def __init__(self,m,l,price,makerFee,takerFee):
		self.m=m
		self.l=l
		self.p=price
		self.mp=price
		self.lp=price
		self.initialPrice = price
		self.adjusted_value_extrema = [1]
		self.lowest_adjusted_value = 1
		self.highest_adjusted_value = 1
		
		self.makerFee = makerFee
		self.takerFee = takerFee
		
		self.a=0.5/price
		self.b=0.5
		
		self.q=numpy.sqrt(l)/(numpy.sqrt(l)-1)

		self.a_i = (self.q-1)*self.a
		self.b_i = (self.q-1)*self.b
		
	def update(self,price):
		self.p = price

		unadjusted = price>=self.mp*self.m or price*self.m<=self.mp or price>=self.lp*self.l or price*self.l<=self.lp
		
		while unadjusted:
			if price*(1-self.makerFee)>=self.mp*self.m:
				if self.lp*self.l<=self.mp*self.m:
					self.lp*=self.l
					self.mp=self.lp
					value = self.a*self.mp+self.b
					value*=(1-self.takerFee*0.25)
					self.a = 0.5*value/self.mp
					self.b = 0.5*value
					self.a_i = (self.q-1)*self.a
					self.b_i = (self.q-1)*self.b

				else:
					self.mp*=self.m
				
					self.v_i = (self.a_i+self.a)*self.mp+self.b_i+self.b
					self.a=self.v_i*0.5/self.mp-self.a_i
					self.b=self.v_i*0.5-self.b_i
			
			elif price*self.m<=self.mp*(1-self.makerFee):
				if self.lp*self.m>=self.mp*self.l:
					self.lp/=self.l
					self.mp=self.lp
					value = self.a*self.mp+self.b
					value*=(1-self.takerFee*0.25)
					self.a = 0.5*value/self.mp
					self.b = 0.5*value
					self.a_i = (self.q-1)*self.a
					self.b_i = (self.q-1)*self.b

				else:
					self.mp/=self.m
				
					self.v_i = (self.a_i+self.a)*self.mp+self.b_i+self.b
					self.a=self.v_i*0.5/self.mp-self.a_i
					self.b=self.v_i*0.5-self.b_i
		 	
			elif price*(1-self.makerFee)>=self.lp*self.l:
				self.lp*=self.l
				self.mp=self.lp
				value = self.a*self.mp+self.b
				value*=(1-self.takerFee*0.25)
				self.a = 0.5*value/self.mp
				self.b = 0.5*value
				self.a_i = (self.q-1)*self.a
				self.b_i = (self.q-1)*self.b

			elif price*self.l<=self.lp:
				self.lp/=self.l
				self.mp=self.lp
				value = self.a*self.mp+self.b
				value*=(1-self.takerFee*0.25)
				self.a = 0.5*value/self.mp
				self.b = 0.5*value
				self.a_i = (self.q-1)*self.a
				self.b_i = (self.q-1)*self.b

			else:
				unadjusted = False
				
		
		value = self.a*price+self.b
		temp = value+0.5-0.5*price/self.initialPrice
		if self.lowest_adjusted_value>temp:
			self.lowest_adjusted_value = temp
			if self.adjusted_value_extrema[-1]<1:
				self.adjusted_value_extrema[-1] = temp
			else:
				self.adjusted_value_extrema.append(temp)

		elif self.highest_adjusted_value<temp:
			self.highest_adjusted_value = temp
			if self.adjusted_value_extrema[-1] > 1:
				self.adjusted_value_extrema[-1] = temp
			else:
				self.adjusted_value_extrema.append(temp)
		


		
	def getValue(self):
		value = self.a*self.p+self.b
		return (value, self.adjusted_value_extrema)

	def processPriceData(self, price_data):
		for p in price_data:
			self.update(p)

def getAdjustedReturns(m,l,price_data,makerFee=0,takerFee=0):
	temp = LiquidityEngine(m,l,price_data[0],makerFee=makerFee,takerFee=takerFee)
	temp.processPriceData(price_data)
	tmp = temp.getValue()
	return [tmp[0]+0.5-0.5*price_data[-1]/price_data[0], tmp[1]]

#endregion

GATHERED_SYMBOLS_PATH = "Data/GatheredSymbols.txt"
PRICE_DATA_PATH = "Data/PriceData/"
JUMPS_PATH = "Data/Observations/Jumps.bin"
CONFIGURATIONS_PATH = "Data/Observations/Configurations.bin"

PROGRESS_PATH = "Data/Observations/Day/LEReturns/Progress.txt"
OBSERVED_SYMBOLS_PATH = "Data/Observations/Day/LEReturns/ObservedSymbols.txt"
OBSERVATIONS_PATH = "Data/Observations/Day/LEReturns/Observations/"

#region

symbols = []
progress = 0

with open(GATHERED_SYMBOLS_PATH, "r") as file:
	for line in file:
		symbols.append(line.strip())

with open(CONFIGURATIONS_PATH, "rb") as file:
	configurations = readUniformDataBin(file, "[(dd)]")[0]

with open(JUMPS_PATH, "rb") as file:
	jumps = readUniformDataBin(file, "[d]")[0]

try:
	with open(PROGRESS_PATH, "r") as file:
		for line in file:
			progress = int(line.strip())
except:
	with open(PROGRESS_PATH, "w") as file:
		progress = 0
		file.write("0\n")
	
#endregion
	
nof_symbols = len(symbols)
nof_confs = len(configurations)
nof_symbol_conf_combos = nof_symbols*nof_confs

previous_symbol_index = -1

print(len(symbols))

for index in range(progress,nof_symbol_conf_combos):

	with open(PROGRESS_PATH, "a") as file:
		file.write(str(index)+"\n")

	conf_index = index%nof_confs
	symbol_index = int((index-conf_index)/nof_confs)
	
	conf = configurations[conf_index]
	
	if  symbol_index != previous_symbol_index:
		
		symbol = symbols[symbol_index]
		
		with open(PRICE_DATA_PATH+symbol+".bin", "rb") as file:
			temp = readUniformDataBin(file, "[q][d]")
			time_data = temp[0]
			price_data = temp[1]
			
		previous = time_data[0]
		gaps = []

		for i in range(len(time_data)):
			temp = time_data[i]
			gap = temp-previous
			if gap>60*60*24*0.5:
				gaps.append(i)
			previous = temp
				
		nof_days = len(gaps)-1
		
		start = price_data[0]
		
		max_jump = max(max(price_data)/start,start/min(price_data))
		
		day_price_datas = []
		max_jumps = []
		
		for i in range(nof_days):
			day_price_data = price_data[gaps[i]:gaps[i+1]]
			day_price_datas.append(day_price_data)
			start = day_price_data[0]
			max_jumps.append(max(max(day_price_data)/start,start/min(day_price_data)))
			
		symbol_path = OBSERVATIONS_PATH + symbol + "/"
			
		try:
			os.mkdir(symbol_path)
		except:
			pass
			
		previous_symbol_index = symbol_index
	
	print()
	print(str(index)+"/"+str(nof_symbol_conf_combos))
	print(symbol)
	print(conf)
		
	data = []
	
	m = conf[0]
	l = conf[1]
	
	for i in range(nof_days):
		day_price_data = day_price_datas[i]
		max_jump = max_jumps[i]
		if m > max_jump:
			data.append(1.0)
			data.append([])
		else:
			data.extend(getAdjustedReturns(m,l,day_price_data))
		
	with open(symbol_path+"conf"+str(conf_index)+".bin", "wb") as file:
		writeUniformDataBin(file, "[d[d]]",[data])
		
	print(str(data[0]))
