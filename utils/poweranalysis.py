# Python program to read CSV file line by line 
# import necessary packages 
import csv 
import numpy
  


measurement_sample_len = []
measurement_us_len = []
measurement_energy = []
measurement_curr = []
measurement_power = []

file = "classifier_370_800_cl1_s_kw6.csv"

# Open file  
with open(file) as file_obj: 
      
    # Create reader object by passing the file  
    # object to reader method 
    reader_obj = csv.reader(file_obj) 

    curr_measurement_sample_len = []
    curr_measurement_us_len = []
    curr_measurement_energy = []
    curr_measurement_curr = []
    curr_measurement_power = []

    prev_idx = 0
      
    # Iterate over each row in the csv  
    # file using reader object 
    for row in reader_obj: 

        if (row[0] == "iteration"):
            continue

        if (row[0] == "" and prev_idx == -1):
            continue

        if (row[0] == ""):
            measurement_sample_len.append(numpy.sum(curr_measurement_sample_len))
            measurement_us_len.append(numpy.sum(curr_measurement_us_len))
            measurement_energy.append(numpy.sum(curr_measurement_energy))
            measurement_curr.append(numpy.average(curr_measurement_curr, weights=curr_measurement_sample_len))
            measurement_power.append(numpy.average(curr_measurement_power, weights=curr_measurement_sample_len))
        
            curr_measurement_sample_len = []
            curr_measurement_us_len = []
            curr_measurement_energy = []
            curr_measurement_curr = []
            curr_measurement_power = []


        elif (prev_idx == 0 and row[3] == 0):
            measurement_sample_len.append(curr_measurement_sample_len[0])
            measurement_us_len.append(curr_measurement_us_len[0])
            measurement_energy.append(curr_measurement_energy[0])
            measurement_curr.append(curr_measurement_curr[0])
            measurement_power.append(curr_measurement_power[0])

            curr_measurement_sample_len = []
            curr_measurement_us_len = []
            curr_measurement_energy = []
            curr_measurement_curr = []
            curr_measurement_power = []

            curr_measurement_sample_len.append(float(row[3]))
            curr_measurement_us_len.append(float(row[4]))
            curr_measurement_energy.append(float(row[7]))
            curr_measurement_curr.append(float(row[5]))
            curr_measurement_power.append(float(row[6]))

        else:
            curr_measurement_sample_len.append(float(row[3]))
            curr_measurement_us_len.append(float(row[4]))
            curr_measurement_energy.append(float(row[7]))
            curr_measurement_curr.append(float(row[5]))
            curr_measurement_power.append(float(row[6]))

        if (row[0] == ""):
            prev_idx = -1
        else:
            prev_idx = int(row[0])

average_sample_len = numpy.average(measurement_sample_len)
average_us_len = numpy.average(measurement_us_len)
average_energy = numpy.average(measurement_energy)
average_curr = numpy.average(measurement_curr)
average_power = numpy.average(measurement_power)

print ("file,sample len,us len,average_energy,average_curr,average_power")
print (f"{file},{average_sample_len},{average_us_len},{average_energy:.3f},{average_curr:.3f},{average_power:.3f}")
