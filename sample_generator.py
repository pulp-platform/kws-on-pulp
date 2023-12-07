import random
import glob
import csv
import shutil
import os

def main():


    wordlist = ['silence','unknwon','yes','no','up','down','left','right','on','off','stop','go']
    csv_columns = ['class', 'label', 'samplelist']
    sampledict = {
    'silence': {'label': 0, 'samplelist': ['','','','','','','','','','']},
    'unknwon': {'label': 1, 'samplelist': []}, # TODO: Populate
    'yes':     {'label': 2, 'samplelist': []},
    'no': {'label': 3, 'samplelist': []},
    'up': {'label': 4, 'samplelist': []},
    'down': {'label': 5, 'samplelist': []},
    'left': {'label': 6, 'samplelist': []},
    'right': {'label': 7, 'samplelist': []},
    'on': {'label': 8, 'samplelist': []},
    'off': {'label': 9, 'samplelist': []},
    'stop': {'label': 10, 'samplelist': []},
    'go': {'label': 11, 'samplelist': []}
    }


    # Parse source dir
    filelist = [f for f in glob.glob("/usr/scratch/sassauna2/cioflanc/dolphinGSC/speech_commands_v0.02/*/*.wav")]
    random.shuffle(filelist)

    shutil.rmtree('./validation')
    os.mkdir('./validation')    

    for file in filelist:
        for word in wordlist:
            if word == file.split('/')[-2]:
           
                shutil.copy(file, '/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/validation/'+word+'_'+ file.split('/')[-1])
                file = '/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/validation/'+word+'_'+ file.split('/')[-1]
                sampledict[word]['samplelist'].append(file)

    # dump list
    csv_file = 'validation.csv'
    with open(csv_file, 'w') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=csv_columns)
        writer.writeheader()
        for datadict in sampledict:
            writer.writerow(sampledict[datadict])


    # simplify - dump all in text
    dumplist = []
    for word in sampledict:
        for elem in sampledict[word]['samplelist']:
            dumplist.append(elem)

    with open('validation.txt', 'w') as f:
        for elem in dumplist:
            f.write(f"{elem}\n")


    # Arrange in .h to include it in app
    with open('validation.h', 'w') as f:
        f.write(f"#ifndef __VALIDATION_H__\n")
        f.write(f"#define __VALIDATION_H__\n")

        classidx = 0
        for word in sampledict:
            f.write(f"char class_{classidx}[1000][100] = {{")
            elemidx = 0
            for elem in sampledict[word]['samplelist']:
                if (elemidx == 9):
                    f.write(f' "{elem}"')
                else:
                    f.write(f' "{elem}",')
            f.write(f"}};\n")
            classidx += 1

        f.write(f"#endif \n")

        



if __name__ == "__main__":
	main()