import numpy as np


def main():

    # Using readlines()
    file = open('onboard_loss_ambulance.log', 'r')

    # Pretrain_loss loss avg:  1.4015226
    # Finetune_loss loss avg:  1.3570989
    # Pretrain_loss loss med:  0.009605
    # Finetune_loss loss med:  0.0046925
    # Diff loss: 0.04442370000000014

    lines = file.readlines()

    margin = 0
    losses = []
    predictions = []
    groundtruths = []
    for line in lines:
        if line.startswith("-----------------------------Loop evaluation"):
            margin = 0
        if (margin == 4):
            if line.startswith('Loss is'):
                losses.append(float(line.split('is ')[1]))
        margin += 1

    pretrain_loss = losses[:10]
    finetune_loss = losses[10:]

    print ("Pretrain_loss loss avg: ", np.average(pretrain_loss))
    print ("Finetune_loss loss avg: ", np.average(finetune_loss))
    print ("Pretrain_loss loss med: ", np.median(pretrain_loss))
    print ("Finetune_loss loss med: ", np.median(finetune_loss))
    print ("Diff loss:", np.average(np.subtract(pretrain_loss, finetune_loss)))


if __name__ == "__main__":
    main()