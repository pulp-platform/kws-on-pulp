############################################################################################## OpenOCD PULP Flashing Session ###################################################################

# PULP flasher ctrl: loads a bin ImageName of size ImageSize to flash at addr 0x0+flash_offset
proc pulp_flasher_ctrl {ImageName ImageSize flash_offset} {
    # Localizes the Status Buffer in L2
    set device_struct_ptr_addr 0x1c003000
    # Reads the address of Status Buffer
    # Produces an array like: array set device_struct_ptr {0 status_buff_addr},
    # where 32 is the number of elements to read from addr and 1 is the number of elements of the array to produce
    mem2array device_struct_ptr 32 $device_struct_ptr_addr 1
    # Sets the pointers to point the correct values (every 4 byte there is a new info)
    set image_ready [expr $device_struct_ptr(0) + 0 ]
    set flash_run [expr $device_struct_ptr(0) + 4 ]
    set flasher_ready [expr $device_struct_ptr(0) + 8 ]
    set buff_ptr_addr [expr $device_struct_ptr(0) + 12 ]
    set max_buff_size [expr $device_struct_ptr(0) + 16 ]
    set flash_addr [expr $device_struct_ptr(0) + 20 ]
    set flash_size [expr $device_struct_ptr(0) + 24 ]
    # The ImageSize is an input of the OpenOCD procedure
    set size [expr $ImageSize]
    # Sets the Host flag which starts the synchronization procedure
    # Takes as first value the addr and as second the value to write
    mww [expr $image_ready] 0x0    
    mww [expr $flash_run] 0x1
    puts "Host: Starting synchronization procedure and opening the flashing session"
    # Reads the address of Flash Buffer in L2
    mem2array buff_ptr 32 $buff_ptr_addr 1
    # Creates the portions of image to flash in HyperFlash
    set curr_offset $flash_offset
    mem2array max_size 32 $max_buff_size 1
    puts "Host: Image size is [expr $size] and max_buff_size is [expr $max_size(0)]"
    while { $size > 0 } {
        # Sets the Host ready flag to busy
        mww [expr $image_ready] 0x0
        # Reads the Board flag and waits until it is ready
        mem2array wait1 32 $flasher_ready 1
        while { [expr $wait1(0) != 1] } {
            mem2array wait1 32 $flasher_ready 1
            sleep 1
        }
        if { $size > $max_size(0) } {
            puts "Host: The remained portion of image [expr $size] is greater than max buffer size [expr $max_size(0)]"
            set curr_size [expr $max_size(0)]
            set size [expr $size - $max_size(0)]
        } else {
                set curr_size [expr $size]
                set size [expr 0]
        }
        # Sets the parameters of this tranfert
        mww [expr $flash_addr] $curr_offset
        mww [expr $flash_size] $curr_size
        # Loads this portion of image
        puts "Host: Loading image"
        load_image $ImageName [expr $buff_ptr(0) - $curr_offset] bin $buff_ptr(0) $curr_size
        # puts "Host: Testing image"
        # test_image $ImageName [expr $buff_ptr(0) - $curr_offset] bin 


        # set fp [open $ImageName r]
        # set file_data [read $fp]
        # close $fp
        # set data [split $file_data "\n"]
        # foreach line $data {
        #     set words [split $line " "]
        #     foreach word $words {
        #         if { $word == floor($word) } {
        #                puts [format %X $word]
        #         }                   
        #     }
        # }
        
        # Increments the offset for the next transfert
        set curr_offset [expr $curr_offset + $curr_size]
        # Sets the Host ready flag to ready
        mww [expr $image_ready] 0x1
        # If is the last transfert the flashing procedure is done
        if { $size == 0 } {
            mww [expr $flash_run] 0x0
        }
    }
    # Reads the Board flag and waits until it is ready
    mem2array wait1 32 $flasher_ready 1
    while { [expr $wait1(0) != 1] } {
        mem2array wait1 32 $flasher_ready 1
        sleep 1
    }
    # Concludes the tranfert
    puts "Host: Closing the flashing session"
        mww [expr $image_ready]  0x0
        mww [expr $flash_run]  0x0
}

# PULP load binary: loads a binary of defined type
proc pulp_jtag_load_binary_and_start {file_name type} {
    puts "Host: Loading binary through JTAG"
    halt
    wait_halt
    load_image $file_name 0x0 $type
    # Sets PC at reset vector addr
    resume 0x1c008080
}

# PULP Flashing Session: Before flashes the pre-compiled binary file (flasher) from path and after flashes image_name using the pulp_flasher_ctrl
proc pulp_flash_raw {image_name image_size path} {
    # Flashes the flasher
    puts "Host: Flashing the flasher"
    pulp_jtag_load_binary_and_start $path/build/pulp/flasher/flasher elf
    sleep 1000
    # Flashes the flash image with the flasher
    puts "Host: Flashing the image with flasher"
    pulp_flasher_ctrl $image_name $image_size 0x0
    sleep 1000
    # Flashes the application
    puts "Host: Flashing the application"    
    pulp_jtag_load_binary_and_start $path/test/build/pulp/main/main elf
    sleep 10000
    shutdown
}
