import collections
import re
import os

def exo1():
    dict = {
        "MPL" : "Montpellier",
        "DUB" : "Dublin",
        "CDG" : "Paris Charles-de-Gaule",
        "ORY" : "Paris Orly",
        "YYZ" : "Toronto",
        "LHR" : "London",
        "LAX" : "Los Angeles"
    }

    print(dict)
    dict["LHR"] = "London Heathrow"
    dict["LCY"] = "London City"
    print(dict)


def exo2():
    liste1 = ["MPL", "DUB", "CDG", "ORY", "YYZ", "LHR", "LAX"]
    liste2 = ["Montpellier", "Dublin", "Paris Charles-de-Gaule", "Paris Orly", "Toronto", "London", "Los Angeles"]
    newDict = dict(zip(liste1, liste2))
    print(newDict)


def exo3():
    my_dict = {
        "MPL" : "Montpellier",
        "DUB" : "Dublin",
        "CDG" : "Paris Charles-de-Gaule",
        "ORY" : "Paris Orly",
        "YYZ" : "Toronto",
        "LHR" : "London",
        "LAX" : "Los Angeles"
    }
    name = input("Entrez le nom de la ville : ")

    val_list = list(my_dict.values())
    key_list = list(my_dict.keys())
    position = val_list.index(name)

    print(key_list[position])


def exo4():
    my_dict = {
        "MPL" : "Montpellier",
        "DUB" : "Dublin",
        "CDG" : "Paris Charles-de-Gaule",
        "ORY" : "Paris Orly",
        "YYZ" : "Toronto",
        "LHR" : "London",
        "LAX" : "Los Angeles"
    }

    sortedDict = collections.OrderedDict(sorted(my_dict.items()))
    newDict = {}
    for k,v in sortedDict.items() : 
        newDict[k] = v
    print(newDict)


def exo5():
    my_dict = {
        "MPL" : "Montpellier",
        "DUB" : "Dublin",
        "CDG" : "Paris Charles-de-Gaule",
        "ORY" : "Paris Orly",
        "YYZ" : "Toronto",
        "LHR" : "London",
        "LAX" : "Los Angeles"
    }

    print(dict(sorted(my_dict.items(), key=lambda item: item[1])))


def adresseMailConforme():
    txt = "The.rai08n@inSpain.com"
    x = re.search("^[\w(-.)]+@[\w(-.)]+(.fr|.com|.org)\Z", txt) 
    if x != None:
        return True
    else:
        return False
#print(adresseMailConforme())


def exo9():
    num1 = "0644175379"
    num2 = "06 44 17 53 79"
    num3 = "06-44-17-53-79"
    num4 = "+33476812847"
    x = re.search("^(\+\d{2}+|0)+[\w(-.)]+", num1)


def exo13():
    file = open('listeaeroports.csv', 'r', encoding="utf-8")
    keys = []
    values = []
    for line in file:
        data = line.split(",")
        data.pop()
        for i in range(3):
            if data[len(data)-1] == '':
                data.pop()
        keys.append(data[0])
        data.pop(0)
        values.append(data)
    newDict = dict(zip(keys, data))
    print(newDict)




exo13()
