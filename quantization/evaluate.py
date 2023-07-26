

def main():

    # read file(s)
    # out_layer10 - CL
    # out_layer11 - BB


    # # read file(s)

    with open("out_layer10.txt") as file_in:
        lines = []
        for line in file_in:
            if (line.startswith('#')):
                continue
            lines.append(line[:-3]) # out_layer10
            # lines.append(line[:-5]) # out_layer11

    with open("batched_outputs_float.txt") as file_in:
        fllines = []
        for line in file_in:
            if (line.startswith('--------')):
                continue
            fllines.append(line[:-3])


    with open("out_layer11_gvsoc_fp32W.txt") as file_in:
        gvlines = []
        for line in file_in:
            gvlines.append(line[:-2])


    with open("batched_outputs_softmax_float.txt") as file_in:
        softmax_float = []
        for line in file_in:
            if (line.startswith('--------')):
                continue
            softmax_float.append(line[:-3])


    with open("batched_outputs_softmax_gvsoc.txt") as file_in:
        softmax_gvsoc = []
        for line in file_in:
            if (line.startswith('--------')):
                continue
            softmax_gvsoc.append(line[:-3])



    # # read file(s)
    # with open("out_layer10_gvsoc_fp32W.txt") as file_in:
    #     gvlines = []
    #     for line in file_in:
    #         gvlines.append(line[:-2])




    # Note: 0 - FQ | 1 - INT | 2 - FP

    # array 1
    array1 = lines[0*12:1*12]


    # array 2
    array2 = gvlines[0*12:1*12] # gvsoc


    # array3
    array3 = fllines[1*12:2*12] # 0 FQ | 1 FP


    array4 = softmax_gvsoc[0*12:1*12]
    array5 = softmax_float[0*12:1*12]


    # scaling factor
    eps_in = 0.1942


    # compute MSE
    err = 0
    norm = 0
    for idx in range (len(array5)):
        err += (float(array5[idx]) - float(array4[idx])) * (float(array5[idx]) - float(array4[idx]))
        norm += float(array5[idx]) * float(array4[idx])

    mse = err/float(len(array5))


    # compute normalized MSE
    nmse = err/norm

    print ("FQ-SoftMax vs GVSOC-SoftMax")
    print ("MSE: ", mse)
    print ("NMSE: ", nmse)

# PyTorch measurements on BACKBONE

# FP-BB vs FQ-BB
# MSE:  0.11716845548557767
# NMSE:  0.27780802193609766

# FP-BB vs INT-BB
# MSE:  0.12920379190728018
# NMSE:  0.31534926847469114

# FQ-BB vs INT-BB
# MSE:  0.008065017685228751
# NMSE:  0.014003023156131302


# FP-BB vs GVSOC-INT-BB
# MSE:  0.12920379190728015
# NMSE:  0.31534926847469114

# _________________________


# PyTorch measurements on CLASSIFIER

# FQ-CL (.int()) vs FP-CL (.int())
# MSE:  6.75
# NMSE:  0.164969450101833

# FQ-CL (.int()) vs INT-CL
# MSE:  264350.9964237934
# NMSE:  68.93994335804855

# FP-CL (.int()) vs INT-CL
# MSE:  266213.87629046
# NMSE:  92.0190588281399

# FP-CL (.int()) vs GVSOC-INT-CL
# MSE:  7.6710902422750005
# NMSE:  0.18190747562378196

# FQ-CL (.int()) vs GVSOC-INT-CL
# MSE:  0.3279419089416668
# NMSE:  0.005868310904889207

# FQ-CL vs GVSOC-INT-CL
# MSE:  0.19665940869110343
# NMSE:  0.0033570744967253174

# FP-CL vs GVSOC-INT-CL
# MSE:  6.384423298651575
# NMSE:  0.14014344072534501

# _________________________


# FQ-SoftMax vs GVSOC-SoftMax - Exponential of Davide
# MSE:  0.05401336115718131
# NMSE:  -44.30015041012826


# FQ-SoftMax vs GVSOC-SoftMax - C Exponential 
# MSE:  0.00027199657666113577
# NMSE:  0.0067581038109658185

# ___________________________

# TODO: Loss

# TODO: Gradients

# TODO: New weights





if __name__ == "__main__":
    main()