

def main():

    with open("loss_lr001_x1washing_nakws.txt") as file:
        lines = []
        for line in file:
            if (line.startswith('Loss is ')):
                lines.append(line)

    pretraining = 0.
    finetuning = 0.


    nlines = len(lines)
    for index in range(0, len(lines)):
        if (index > 0 and index < 11):
            print (str(index) +": " + lines[index][8:])
            pretraining += float(lines[index][8:])
        elif (index > (nlines-11)):
            print (str(index) +": " + lines[index][8:])
            finetuning += float(lines[index][8:])    

    print ("pretraining: ", pretraining/10)
    print ("finetuning: ", finetuning/10)
    print ("loss diff: ", pretraining/10-finetuning/10)





if __name__=="__main__":
    main()