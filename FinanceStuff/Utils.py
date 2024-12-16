import struct

from numpy._typing import _128Bit

def getListLength(file):
	n = 0
	num = 128
	while num>=128:
		num = int.from_bytes(file.read(1))
		n*=128
		n+=num%128
	#print("read")
	#print(n)
	return n
	
def writeListLength(file,n):
	num = n
	int_lst = [num%128]
	num-=num%128
	while num > 0:
		num=int(num/128)
		int_lst.insert(0,num%128+128)
		num-=num%128
	
	#print(n)
	#print(int_lst)

	for i in int_lst:
		file.write(i.to_bytes(1))
		
def getFormatStringProperties(format_string):
	length = len(format_string)

	list_sizes = {}
	list_starts = []
	list_depth = -1
	list_counts = []
	no_list_within = {}
	list_symbols_temp = ""
	list_symbols = {}
	list_byte_sizes_temp = 0
	list_byte_sizes = {}
	list_closes = {}
	
	for i in range(length):
		char = format_string[i]

		match char:
			case "[" | "(":
				if list_depth>=0:
					list_counts[list_depth]+=1
					no_list_within[list_starts[list_depth]] = False
				
				list_symbols_temp = ""
				list_byte_sizes_temp = 0
				list_counts.append(0)
				list_starts.append(i+1)
				no_list_within[i+1] = True
				list_depth+=1
				
			case "]" | ")":
				list_closes[list_starts[-1]] = i

				if no_list_within[list_starts[-1]]:
					list_start = list_starts[-1]
					list_symbols[list_start] = list_symbols_temp
					list_byte_sizes[list_start] = list_byte_sizes_temp
				if list_counts[-1] > 0:
					list_sizes[list_starts.pop()] = list_counts.pop()
				else:
					list_sizes[list_starts.pop()] = 1
				list_depth-=1
			case _:
				list_symbols_temp+=char
				list_byte_sizes_temp+=struct.calcsize(char)
				if list_depth>=0:
					list_counts[list_depth]+=1
			
	return (length, list_sizes, no_list_within, list_symbols, list_byte_sizes, list_closes)
		
def writeUniformDataBin(file, format_string, data):
	temp = getFormatStringProperties(format_string)
	
	length = temp[0]
	list_sizes = temp[1]
	no_list_within = temp[2]
	list_symbols = temp[3]
	list_closes = temp[5]
		
	i = 0
	list_length = len(data)
	list_progress = 0
	list_lengths = [list_length]
	list_progs = [list_progress]
	list_starts = [0]
	list_contents = [data]
	list_depth = 0
	
	while i<length:
		#print(list_contents[list_depth])
		char = format_string[i]
		#print(char)
		match char:
			case "q":
				file.write(struct.pack("q",list_contents[list_depth][list_progress]))
				i+=1
				list_progress+=1
			case "d":
				file.write(struct.pack("d",list_contents[list_depth][list_progress]))
				i+=1
				list_progress+=1
			case "[":
				i+=1
				list_progs[list_depth] = list_progress
				
				list_contents.append(list_contents[list_depth][list_progress])
				list_depth+=1
				
				list_length = len(list_contents[list_depth])
				list_lengths.append(list_length)
				list_starts.append(i)
				list_progress = 0
				list_progs.append(list_progress)
				
				writeListLength(file, int(list_length/list_sizes[i]))
				
				if list_length == 0:
					i = list_closes[i]
				elif no_list_within[i]:
					form = str(list_length)+list_symbols[i]
					#print(form)
					file.write(struct.pack(form,*list_contents[list_depth]))
					list_progress = list_length
					
					i = list_closes[i]
						
			case "]":
				if list_length>list_progress:
					i=list_starts[list_depth]
				else:
					list_contents.pop()
					list_starts.pop()
					list_lengths.pop()
					list_length = list_lengths[-1]
					list_progs.pop()
					list_progress = list_progs[-1]
					list_progress+=1
					list_depth-=1
					i+=1
					
			case "(":
				i+=1
				list_progs[list_depth] = list_progress
				
				list_contents.append(list_contents[list_depth][list_progress])
				list_depth+=1
				
				list_length = 0
				list_lengths.append(list_length)
				list_starts.append(i)
				list_progress = 0
				list_progs.append(list_progress)
				
			case ")":
				list_contents.pop()
				list_starts.pop()
				list_lengths.pop()
				list_length = list_lengths[-1]
				list_progs.pop()
				list_progress = list_progs[-1]
				list_progress+=1
				list_depth-=1
				i+=1

def readUniformDataBin(file, format_string):
	temp = getFormatStringProperties(format_string)
	
	length = temp[0]
	no_list_within = temp[2]
	list_symbols = temp[3]
	list_byte_sizes = temp[4]
	list_closes = temp[5]
	
	i = 0
	list_length = 1
	list_lengths = [list_length]
	list_starts = [0]
	list_contents = [[]]
	list_depth = 0
	
	while i<length:
		#print(i)
		char = format_string[i]
		match char:
			case "q":
				list_contents[list_depth].append(struct.unpack("q",file.read(8))[0])
				i+=1
			case "d":
				list_contents[list_depth].append(struct.unpack("d",file.read(8))[0])
				i+=1
			case "[":
				i+=1
				list_lengths[list_depth] = list_length
				
				list_length = getListLength(file)
				
				list_lengths.append(list_length)
				list_starts.append(i)
				list_contents.append([])
				list_depth += 1
				
				if list_length == 0:
					i = list_closes[i]
				elif no_list_within[i]:
					form = str(list_length)+list_symbols[i]
					#print(form)
					temp = struct.unpack(form, file.read(list_length*list_byte_sizes[i]))
					for tmp in temp:
						list_contents[list_depth].append(tmp)
					list_length = 0
					i = list_closes[i]
			case "]":
				if list_length>1:
					list_length-=1
					i=list_starts[list_depth]
				else:
					list_depth-=1
					list_contents[list_depth].append(list_contents.pop())
					list_starts.pop()
					list_lengths.pop()
					list_length = list_lengths[-1]
					i+=1
					
			case "(":
				i+=1
				list_lengths[list_depth] = list_length
				
				list_length = 0
				
				list_lengths.append(list_length)
				list_starts.append(i)
				list_contents.append([])
				list_depth += 1
				
			case ")":
				list_depth-=1
				list_contents[list_depth].append(list_contents.pop())
				list_starts.pop()
				list_lengths.pop()
				list_length = list_lengths[-1]
				i+=1
				
	return list_contents[0]			


"""data = [[],[1.0003]]

lst = []

for i in range(301):
	lst.append(i)
	if True:
		data[0].append([i,lst.copy()])

file_format = "[(q[q])](d)"

with open("test.bin", "wb") as file:
	writeUniformDataBin(file, file_format,data)
	
with open("test.bin", "rb") as file:
	print(readUniformDataBin(file, file_format))"""
