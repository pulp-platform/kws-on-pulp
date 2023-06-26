import random
import glob
import csv


def main():


    wordlist = ['silence','unknwon','yes','no','up','down','left','right','on','off','stop','go']
    csv_columns = ['class', 'label', 'samplelist']
    sampledict = {
    'silence': {'label': 0, 'samplelist': ['','','','','','','','','','']},
    'unknwon': {'label': 1, 'samplelist': []},
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
    filelist = [f for f in glob.glob("/home/cioflanc/bonsapps_eenakws/maintain/eenakws_aiasset_v2/bonseyes_EENAKWS/data/speech_commands_v2/datatool/sample_files/*.wav")]
    random.shuffle(filelist)

    for file in filelist:
        for word in wordlist:
            if word in file.split('/')[-1].split('_')[0]:
                if (len(sampledict[word]['samplelist']) == 10):
                    continue
                else:
                    # print (sampledict[word]['samplelist'])
                    sampledict[word]['samplelist'].append(file)

    # dump list
    csv_file = 'utterances.csv'
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

    with open('utterances.txt', 'w') as f:
        for elem in dumplist:
            f.write(f"{elem}\n")



if __name__ == "__main__":
	main()