# Python module and example written by Pierretsc (https://github.com/Pierretsc), small updates by FokkeB

from balise_api import encode_telegram

line = "8010011CB80002421989DCCFD8111D193FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC"

res = encode_telegram(line, max_cpu=0, calc_all=False)
print("Résult :")
print(res[0])