import numpy as np


def main():

    # Using readlines()
    file = open('loss_lr001_x1cafeteria_nlkws.txt', 'r')
    lines = file.readlines()

    margin = 0
    losses = []
    predictions = []
    groundtruths = []
    for line in lines:
        if line.startswith("-----------------------------Loop evaluation"):
            margin = 0
        if (margin == 1):
            # The uttered keyword was: up (4).
            if line.startswith("Tested input:"):
                groundtruths.append(line.split("tinytest/")[1].split("_")[0])
        if (margin == 12):
            if line.startswith('Loss is'):
                losses.append(float(line.split('is ')[1]))
        if (margin == 13):
            # The uttered keyword was: up (4).
            if line.startswith("The uttered keyword was"):
                predictions.append(line.split(": ")[1].split(" (")[0])
        margin += 1

    pretrain_loss = losses[:10]
    finetune_loss = losses[10:]

    print ("Pretrain_loss loss avg: ", np.average(pretrain_loss))
    print ("Finetune_loss loss avg: ", np.average(finetune_loss))
    print ("Pretrain_loss loss med: ", np.median(pretrain_loss))
    print ("Finetune_loss loss med: ", np.median(finetune_loss))
    print ("Diff loss:", np.average(np.subtract(pretrain_loss, finetune_loss)))


    pretrain_acc = 0.
    for x, y in zip(predictions[:10], groundtruths[:10]):
        if (x == y):
            pretrain_acc += 1
    pretrain_acc = pretrain_acc/len(predictions[:10]) 

    finetune_acc = 0.
    for x, y in zip(predictions[10:], groundtruths[10:]):
        if (x == y):
            finetune_acc += 1
    finetune_acc = finetune_acc/len(predictions[10:]) 

    print ("Pretrain_acc: ", pretrain_acc)
    print ("Finetune_acc: ", finetune_acc)
    print ("Diff: ", finetune_acc - pretrain_acc)



if __name__ == "__main__":
    main()