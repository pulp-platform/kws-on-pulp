import numpy as np


def main():

    # Using readlines()
    file = open('loss_lr001_x1washing_nlkws.txt', 'r')
    lines = file.readlines()

    margin = 0
    losses = []
    for line in lines:
        if line.startswith("-----------------------------Loop evaluation"):
            margin = 0

        if (margin == 12):
            if line.startswith('Loss is'):
                losses.append(float(line.split('is ')[1]))

        margin += 1


    pretrain = losses[:10]
    finetune = losses[10:]


    print ("Pretrain loss: ", np.average(pretrain))
    print ("Finetune loss: ", np.average(finetune))
    print ("Diff loss:", np.average(np.subtract(pretrain, finetune)))


if __name__ == "__main__":
    main()