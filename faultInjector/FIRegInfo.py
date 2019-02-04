import collections
import numpy as np
import pdb
import random

registerInfo = {
'FToDC': { 'fields': collections.OrderedDict({'pc': {'start': 0, 'width':32}, 'instruction': {'start': 32, 'width':32}, 'realinstruction': {'start': 64, 'width':1}, 'nextpc': {'start': 65, 'width':32}})
         , 'width': 97, 'id':0 },
'DCToEX': { 'fields': collections.OrderedDict({'pc': {'start': 0, 'width':32}, 'opcode': {'start': 32, 'width':7}, 'funct7': {'start': 39, 'width':7}, 'funct3': {'start': 46, 'width':3}, 'rd': {'start': 49, 'width':5},
 'realinstruction': {'start': 54, 'width':1}, 'lhs': {'start': 55, 'width':32}, 'rhs': {'start': 87, 'width':32}, 'datac': {'start': 119, 'width':32}, 'forward_lhs': {'start': 151, 'width':1},
 'forward_rhs': {'start': 152, 'width':1}, 'forward_datac': {'start': 153, 'width':1}, 'forward_mem_lhs': {'start': 154, 'width':1}, 'forward_mem_rhs': {'start': 155, 'width':1}, 'forward_mem_datac': {'start': 156, 'width':1},
 'csr': {'start': 157, 'width':1}, 'csrid': {'start': 158, 'width':12}, 'external': {'start': 170, 'width':1}})
         , 'width':171, 'id':1 },
'EXToMEM': { 'fields': collections.OrderedDict({'pc': {'start':0, 'width':32}, 'result': {'start': 32, 'width':32}, 'rd': {'start': 64, 'width':5}, 'opcode': {'start': 69, 'width':7}, 'funct3': {'start': 76, 'width':3}, 'realinstruction': {'start': 79, 'width':1},
 'external': {'start': 80, 'width':1}, 'csr': {'start': 81, 'width':1}, 'csrid': {'start': 82, 'width':12}, 'datac': {'start': 94, 'width':32}})
         , 'width':126, 'id':2 },
'MEMToWB': { 'fields': collections.OrderedDict({'result': {'start': 0, 'width':32}, 'rd': {'start': 32, 'width':5}, 'realinstruction': {'start': 37, 'width':1}, 'csr': {'start': 38, 'width':1}, 'csrid': {'start': 39, 'width':12},
 'rescsr': {'start': 51, 'width':32}})
         , 'width':83, 'id':3 },
'PC': {'width':32, 'id':4},
'RF0': {'width':32, 'id':5},
'RF1': {'width':32, 'id':6},
'RF2': {'width':32, 'id':7},
'RF3': {'width':32, 'id':8},
'RF4': {'width':32, 'id':9},
'RF5': {'width':32, 'id':10},
'RF6': {'width':32, 'id':11},
'RF7': {'width':32, 'id':12},
'RF8': {'width':32, 'id':13},
'RF9': {'width':32, 'id':14},
'RF10': {'width':32, 'id':15},
'RF11': {'width':32, 'id':16},
'RF12': {'width':32, 'id':17},
'RF13': {'width':32, 'id':18},
'RF14': {'width':32, 'id':19},
'RF15': {'width':32, 'id':20},
'RF16': {'width':32, 'id':21},
'RF17': {'width':32, 'id':22},
'RF18': {'width':32, 'id':23},
'RF19': {'width':32, 'id':24},
'RF20': {'width':32, 'id':25},
'RF21': {'width':32, 'id':26},
'RF22': {'width':32, 'id':27},
'RF23': {'width':32, 'id':28},
'RF24': {'width':32, 'id':29},
'RF25': {'width':32, 'id':30},
'RF26': {'width':32, 'id':31},
'RF27': {'width':32, 'id':32},
'RF28': {'width':32, 'id':33},
'RF29': {'width':32, 'id':34},
'RF30': {'width':32, 'id':35},
'RF31': {'width':32, 'id':36},
'CoreCtrl': {'width':205, 'id':37}
}

coreAreas_names = ['sequential', 'comb_pipeline', 'comb_other']    #comb_other -> coreCtrl, cache control
#coreAreas_probas = [0.338581382365307417, 0.49667440581689906891, 0.16474421181779351409]
coreAreas_probas = [0.302387984, 0.647612016, 0.05]

pipelineAreas_names = ['FToDC', 'DCToEX', 'EXToMEM', 'MEMToWB', 'WriteBack']
pipelineAreas_probas = [0.065232138, 0.145714051, 0.690445829, 0.049792418, 0.048815564]


def getRegisterList():
    return registerInfo.keys()

def getRegisterID(name):
    return registerInfo[name]['id']

def getTotalWidth():
    total = 0
    for reg in registerInfo:
        total += getRegisterWidth(reg)
    return total

def getFieldPosition(registerName, fieldName):  #returns the position in the given register, returns -1 if field not found
    for i in range(len(registerInfo[registerName]['fields'])):
        if fieldName == list(registerInfo[registerName]['fields'].keys())[i]:
            #print("Matched " + fieldName + " to : " + list(registerInfo[registerName]['fields'].keys())[i] + " at index " + str(i))
            return i
    return -1

def getBitPosition(registerName, fieldName, bitOffset):
    return registerInfo[registerName][fieldName]['start'] + bitOffset

def getRegisterWidth(registerName):
    return registerInfo[registerName]['width']

def getBitField(registerName, bitPosition):
    sum=0
    for key in registerInfo:
        if (key != 'width') and (key != 'id'):
            if (bitPosition >= sum) and (bitPosition < sum+registerInfo[key]['width']):
                return key
            sum += registerInfo[key]['width']

def stripFieldNameList(nameList):
    outputList = []
    for f in nameList:
        #if (f != '') and ("core_REG" not in f):
        outputList.append('_'.join(f.split('_')[2:-2]))  #some fields have _ in their names...
    return outputList


#The patternList is a list of the different bits associated to the register names given in nameList, widths not checked
#returns a list of strings containig the error pattern
def reorderPattern(registerName, patternList, nameList):
    outputPattern =[None]*len(registerInfo[registerName]['fields'])
    processedNameList = stripFieldNameList(nameList)
    '''
    #remove the empties of the processedNameList
    while '' in processedNameList:
        processedNameList.remove('')
        '''
    #sum_actual = 0
    #sum_theo = 0
    #print()
    #print('------------------------------------------------------')
    for i in range(0, len(processedNameList)):
        #pdb.set_trace()
        pos = getFieldPosition(registerName, processedNameList[i].lower())
        if(pos > -1):
            #sum_actual += len(patternList[i])
            #sum_theo += registerInfo[registerName]['fields'][processedNameList[i].lower()]['width']
            #print(processedNameList[i].lower() + ' width : ' + str(registerInfo[registerName]['fields'][processedNameList[i].lower()]['width']) )
            #print(processedNameList[i].lower() + ' actual width : ' + str(len(patternList[i])) + " --- origin list register : " + nameList[i])
            outputPattern[pos] = patternList[i]
    #print('actual total reg pattern width : ' + str(sum_actual) + " theoretical reg width : " + str(sum_theo))
    return outputPattern

#Takes an ordered sub patterns list as the input and returns a bit positions list in the register
def patternListToBitPositions(patternList):
    pos = []
    patternString = ''.join(patternList)
    for n in range(len(patternString)):
        if patternString[n] == '1':
            pos.append(n)
    return pos

#Takes an ordered pattern as the input and returns a bit positions list in the register
def patternToBitPositions(pattern):
    pos = []
    for n in range(len(pattern)):
        if pattern[n] == '1':
            pos.append(n)
    return pos

#normalize all the probas for all the registers patterns sets
def unifyPatternProbas(errorBase):
    for reg in errorBase:
        histSum = 0
        for i in errorBase[reg][1]:
            histSum += i
        for i in range(len(errorBase[reg][1])):
            errorBase[reg][1][i] /= histSum

#Returns a pattern selected function of the provided register histogram, return a list of bits
def getPattern(registerName, errorBase):
    pattern = np.random.choice(errorBase[registerName][0], p=errorBase[registerName][1])
    return pattern

def selectRegion():
    region = np.random.choice(coreAreas_names, p=coreAreas_probas)
    if region == 'comb_pipeline':
        region = np.random.choice(pipelineAreas_names, p=pipelineAreas_probas)
    return region

def selectRegister():
    return random.choice(list(registerInfo))

def selectRFRegister():
    return 'RF' + str(random.randint(0,31))

def getrandomBitsinRegister(registerName, numBits):
    #get numbits random positions within the register
    return random.sample(range(getRegisterWidth(registerName)), numBits)
