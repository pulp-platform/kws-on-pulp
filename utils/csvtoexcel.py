import csv

with open("240_650/summary_240_650_curated.csv") as file_obj:
    reader_obj = csv.reader(file_obj)
    for row in reader_obj: 

        if row[0].startswith("240"):
            row[1] = str(round(float(row[1]), 3)).replace(".", ",")
            row[2] = str(round(float(row[2]), 3)).replace(".", ",")
            row[3] = str(round(float(row[3]), 3)).replace(".", ",")
            row[4] = str(round(float(row[4]), 3)).replace(".", ",")
            row[5] = str(round(float(row[5]), 3)).replace(".", ",")

        print (f"{row[0]};{row[1]};{row[2]};{row[3]};{row[4]};{row[5]};")
