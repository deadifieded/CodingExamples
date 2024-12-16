from Utils import readUniformDataBin, writeUniformDataBin
import matplotlib.pyplot as plt
import numpy as np
import time

PRICE_PATH = "C:/Users/User/source/repos/PythonApplication1/PythonApplication1/Data/PriceData/"

SYMBOLS_PATH = "C:/Users/User/source/repos/PythonApplication1/PythonApplication1/Data/GatheredSymbols.txt"

CORRELATION_RESULTS_PATH = "Data/CorrelationMatrix/Matrixes/"

CORRELATION_PROGRESS_PATH = "Data/CorrelationMatrix/progress.txt"

CORRELATION_BACKUP_PROGRESS_PATH = "Data/CorrelationMatrix/Backup/"

symbols = []

with open(SYMBOLS_PATH, "r") as file: 
    for l in file.readlines():
        symbols.append(l.strip())

#print(symbols)
start_time = time.time()
MEMOISED_BOUNDS = {}        

def calculateBounds(lst, lower, upper, sym_set_index = -1):
    if sym_set_index != -1:
        if sym_set_index in MEMOISED_BOUNDS:
            return MEMOISED_BOUNDS[sym_set_index]

    length = len(lst)
    
    upper_index = length-1
    lower_index = 0
    upper_inner_index = 0
    lower_inner_index = 0

    if lst[0]>lower:
        raise("the floor")
    if lst[-1]<upper:
        raise("the roof")
    
    while True:
        a = int((upper_index+lower_index)/2)
        #print("0 " + str(a))
        
        if a == upper_index or a == lower_index:
            MEMOISED_BOUNDS[sym_set_index] = (a,a)
            return (a,a)

        n = lst[a]

        if n >= upper:
            upper_index = a
        elif n <= lower:
            lower_index = a
        else:
            lower_inner_index = a
            upper_inner_index = a
            break
        
    while True:
        a = int((upper_inner_index+upper_index)/2)
        #print("1 " + str(upper_index - upper_inner_index))
        
        if lst[a]<upper:
            upper_inner_index = a
        else:
            upper_index = a
            
        if upper_index == upper_inner_index+1:
            break
            
    while True:
        b = int((lower_inner_index+lower_index)/2)
        #print("2 " + str(b))
        
        if lst[b]>lower:
            lower_inner_index = b
        else:
            lower_index = b
            
        if lower_index +1 == lower_inner_index:
            MEMOISED_BOUNDS[sym_set_index] = (b,a)
            return (b,a)
            
def function1(x):
    return x+1

def function2(x):
    return 0

def getSymbolData(symbol):
    with open(PRICE_PATH +  symbol + ".bin", "rb") as file:
        temp = readUniformDataBin(file, "[q][d]")
    return temp

def getNextRandomSymbol():
    if len(symbols) == 0:
        raise("no more symbols")
    n = np.random.randint(len(symbols))
    return symbols.pop(n)

def getVariance(x):
    l = len(x)
    mean = sum(x)/l
    x_squared_sum = 0
    for i in range(x):
        x_squared+=i**2
    
    return x_squared_sum/l - mean**2

LOWEST_ACCEPTED = (2021.5-1970)*365*24*60*60

HIGHEST_ACCEPTED = (2023.5-1970)*365*24*60*60

CHUNK_SIZE = 10

NUM_CHUNKS = 100

NUM_SYMBOLS = CHUNK_SIZE*NUM_CHUNKS

"""for i in range(progress*CHUNK_SIZE):
    corr_row = []
    for j in range(progress*CHUNK_SIZE):
        if i == j:
            corr_row.append(float(1))
        else:
            corr_row.append(float(0))
        
    corr_lists.append(corr_row)
    
corr_matrix = corr_lists"""
#corr_matrix = np.array(corr_lists)


symbol_set = []

try:
    with open(CORRELATION_PROGRESS_PATH, "r") as file:
        for line in file.readlines():
            symbol = line.strip()
            symbol_set.append(symbol)
            symbols.remove(symbol)
        progress = len(symbol_set)
    
    with open(CORRELATION_RESULTS_PATH+"matrix"+str(progress)+".bin", "rb") as file:
        corr_matrix = readUniformDataBin(file,"[[d]]")[0]

    if len(corr_matrix)!=progress:
        raise("all hell")
except:
    progress = 0
    with open(CORRELATION_RESULTS_PATH+"matrix"+str(progress)+".bin","wb") as file:
        pass
    corr_matrix = []

    with open(CORRELATION_PROGRESS_PATH, "w") as file:
        pass
        
if progress%CHUNK_SIZE!=0:
    print("not divisible into chunks")
    print((-progress)%chunk_size, end = " ")
    print("more needed")
    raise("poop")

starting_h = int(progress/CHUNK_SIZE)

print(symbol_set)
print()
print(corr_matrix)
print()

for h in range(starting_h, NUM_CHUNKS):

    for i in range(CHUNK_SIZE):
        symbol_set.append(getNextRandomSymbol())

    print(symbol_set)
    print()

    corr_lists = []

    for i in range(progress+CHUNK_SIZE):
        corr_row = []
        for j in range(progress+CHUNK_SIZE):
            if i == j:
                corr_row.append(float(1))

            elif i < progress and j < progress:
                corr_row.append(corr_matrix[i][j])
            
            else:
                corr_row.append(float(0))
        
        corr_lists.append(corr_row)
    
    corr_matrix = corr_lists

    print(corr_matrix)
    print()

    symbols1 = symbol_set[h*CHUNK_SIZE:h*CHUNK_SIZE+CHUNK_SIZE]
    symbol_data1 = []
    
    for j in range(len(symbols1)):
            while True:    
                temp = getSymbolData(symbols1[j])
                if temp[0][0]>LOWEST_ACCEPTED or temp[0][-1]<HIGHEST_ACCEPTED:
                    symbol = getNextRandomSymbol()
                    symbols1[j]=symbol
                    symbol_set[h*CHUNK_SIZE+j] = symbol
                    print("Swapped Symbol:")
                    print(symbol_set)
                    print()
                else:
                    break
            timestamps = temp[0]
            prices = []

            for p in temp[1]:
                prices.append(np.log(p))
                
            symbol_data1.append([timestamps,prices])
    
    for i in range(0,h+1):

        func = function2

        if i == h:
            symbols2 = symbols1
            symbol_data2 = symbol_data1
            func = function1
        else:
            symbols2 = symbol_set[i*CHUNK_SIZE:i*CHUNK_SIZE+CHUNK_SIZE]
            symbol_data2 = []
            for k in range(len(symbols2)):
                while True:    
                    temp = getSymbolData(symbols2[k])
                    if temp[0][0]>LOWEST_ACCEPTED or temp[0][-1]<HIGHEST_ACCEPTED:
                        symbol = getNextRandomSymbol()
                        symbols2[k]=symbol
                        symbol_set[i*CHUNK_SIZE+k] = symbol
                        print("Swapped Symbol:")
                        print(symbol_set)
                        print()
                    else:
                        break
                timestamps = temp[0]
                prices = []

                for p in temp[1]:
                    prices.append(np.log(p))
                    
                symbol_data2.append([timestamps,prices])
            
        print()
        print("Chunk ", end = " ")
        print(h, end = " ")
        print(i)
        print(symbols1, end = " ")
        print(symbols2)
        print()
            
        for j in range(CHUNK_SIZE):
            timestamps1 = symbol_data1[j][0]
            pricedata1 = symbol_data1[j][1]
            length1 = len(timestamps1)
            for k in range(func(j),CHUNK_SIZE):
                print("I", end = "")
                proximity_threshhold = 30
                timestamps2 = symbol_data2[k][0]
                pricedata2 = symbol_data2[k][1]
                length2 = len(timestamps2)
    
                mean1 = 0
                mean2 = 0
                
                multiple_mean = 0
    
                mean_squared1 = 0
                mean_squared2 = 0
    
                num = 0
                
                (a_lower, a_upper) = calculateBounds(timestamps1,LOWEST_ACCEPTED,HIGHEST_ACCEPTED, h*CHUNK_SIZE+j)
                (b_lower, b_upper) = calculateBounds(timestamps2,LOWEST_ACCEPTED,HIGHEST_ACCEPTED, i*CHUNK_SIZE+k)
                
                a = a_lower+1
                b = b_lower+1
                
                x_points = []
                y_points = []
    
                while a <  a_upper and b < b_upper:
                    ts1 = timestamps1[a]
                    ts2 = timestamps2[b]

                    if timestamps1[a-1] == ts1:
                        a+=1
                        continue
                    if timestamps2[b-1] == ts2:
                        b+=1
                        continue
                    
                    prev_time2 = ts2
        
                    if ts1 > ts2 + proximity_threshhold:
                        b+=1
            
                    elif ts1+ proximity_threshhold < ts2:
                        a+=1
            
                    else:
                        """x_points.append(pricedata1[a])
                        y_points.append(pricedata2[b])"""
                        multiple_mean+=pricedata1[a]*pricedata2[b]
                        mean1+=pricedata1[a]
                        mean_squared1+=pricedata1[a]**2
                        mean2+=pricedata2[b]
                        mean_squared2+=pricedata2[b]**2
                        num+=1
            
                        a+=1
                        b+=1
            
                if num == 0:
                    print(" no overlap")
                    continue
            
                multiple_mean/=num
                mean1/=num
                mean2/=num
                mean_squared1/=num
                mean_squared2/=num
                
                x = float((multiple_mean-mean1*mean2)/(np.sqrt((mean_squared1-mean1**2)*(mean_squared2-mean2**2))))
                #x = round(x,2)

                corr_matrix[h*CHUNK_SIZE+j][i*CHUNK_SIZE+k] = x
                corr_matrix[i*CHUNK_SIZE+k][h*CHUNK_SIZE+j] = x
                #print(h*CHUNK_SIZE+j, end = " ")
                #print(i*CHUNK_SIZE+k, end = " ")
                #print(x)
                #print(type(x))
                #print(num)
                #print(corr_matrix)
                """if True:
                    
                    prcdta1 = []
                    prcdta2 = []

                    for p in pricedata1[a_lower+1:a_upper]:
                        prcdta1.append((p-mean1)/np.sqrt(mean_squared1-mean1**2))
                        
                    for p in pricedata2[b_lower+1:b_upper]:
                        prcdta2.append((p-mean2)/np.sqrt(mean_squared2-mean2**2))

                    #plt.plot(x_points, y_points, "ro")
                    plt.plot(timestamps1[a_lower+1:a_upper], prcdta1)
                    plt.plot(timestamps2[b_lower+1:b_upper], prcdta2)
                        
                    plt.show()"""
        print()         
    #record results and update progress
    progress+=CHUNK_SIZE

    with open(CORRELATION_RESULTS_PATH+"matrix"+str(progress)+".bin", "wb") as file:
        writeUniformDataBin(file,"[[d]]", [corr_matrix])

    with open(CORRELATION_PROGRESS_PATH, "w") as file:
        for symbol in symbol_set:
            file.write(symbol+"\n")

    with open(CORRELATION_BACKUP_PROGRESS_PATH+"backup"+str(progress)+".txt", "w") as file:
        for symbol in symbol_set:
            file.write(symbol+"\n")
    print()
    print(corr_matrix)
    print()
    print(symbol_set)
print()
print("Finished!")
