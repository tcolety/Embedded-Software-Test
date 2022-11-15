#include "emb_test.h"
#include <stdbool.h>
#include <stdio.h>

#define STATUS_BITMASK      0b1000
#define FLAG_C_BITMASK      0b0100
#define FLAG_B_BITMASK      0b0010
#define FLAG_A_BITMASK      0b0001

#define PRINT_ENUM                1

enum{
    statusBits,
    upperValue,
    lowerValue,
}type;

const uint8_t input_data[12] =
{
    0xF0, 0x24, 0xF8, 0xC0, 0x4B, 0x1B,
    0x68, 0x4F, 0xF0, 0xFF, 0x31, 0x18,
};

bool calculate_parity(control_channel_t* data){
    uint16_t add, sum = data->flag_a + data->flag_b + data->flag_c + data->status;
    uint8_t mask = 0b10000000, val = data->value;
    bool parity;

    //sums value one bit at a time with mask
    for(int i=0; i<8; i++){
        add = 0;
        add = ((val & mask) >> (7-i)); 
        sum = sum + add;
        mask = mask >> 1;
    }
    parity = (sum & 0b1);
    return parity;
}

void read_channels(void* data_in, void* data_out)
{
    uint8_t index=0, val=0, hex;
    uint8_t part1, part2; //spits each byte into 4 bits
    uint8_t type = statusBits;     //0 or 1 = value,  2 = status

    //casting void* ptr to uint8_t to be able to index
    uint8_t *input_arr = (uint8_t *) data_in;
    control_channel_t *output_arr = (control_channel_t *) data_out;

    int size_of_arr = 12;
    for(int i=0; i<size_of_arr; i++){
        part1 = (input_arr[i] & (0xF0)) >> 4;
        part2 = input_arr[i] & (0x0F);

        for(int j=0; j<2; j++){
            hex = j ? part2 : part1;
            
            switch(type){
                case statusBits:                 //handles [8:11] bits of channel
                                                //[8] = flag_a  [9] = flag_b  [10] = flag_c  [11] = status  
                    output_arr[index].status = (hex & STATUS_BITMASK) >> 3;
                    output_arr[index].flag_c = (hex & FLAG_C_BITMASK) >> 2;
                    output_arr[index].flag_b = (hex & FLAG_B_BITMASK) >> 1;
                    output_arr[index].flag_a = (hex & FLAG_A_BITMASK);
                    
                    type = upperValue;
                    break;
                                        
                case upperValue:                 //handles [4:7] bits of channel
                                                //higher bits of value
                    val = (hex << 4) | val;
                    type = lowerValue;
                    break;
                                       
                case lowerValue:                 //handles [0:3] bits of channel
                                                //lower bits of value
                    val = val | hex;
                    output_arr[index].value = val;
                    val = statusBits;

                    //determining parity
                    output_arr[index].parity = calculate_parity(&output_arr[index]);
                    index++;  

                    type = 0;
                    break; 

                default:
                    printf("ERROR");
                }
        }
    }
}

void print_output(control_channel_t* channels){
    
    for(int i=0; i<8; i++){
        printf("CHANNEL %d\n", i);
        printf("value: %x\n", channels[i].value);   

        #ifdef PRINT_ENUM
        printf("status: ");
        channels[i].status ? printf("CTRL_STAT_ERR\n") : printf("CTRL_STAT_OK\n");
        printf("flag A: ");
        channels[i].flag_a ? printf("CTRL_FLAG_SET\n") : printf("CTRL_FLAG_CLR\n");
        printf("flag B: ");
        channels[i].flag_b? printf("CTRL_FLAG_SET\n") : printf("CTRL_FLAG_CLR\n");
        printf("flag C: ");
        channels[i].flag_c? printf("CTRL_FLAG_SET\n") : printf("CTRL_FLAG_CLR\n");
        printf("parity: ");
        channels[i].parity? printf("CTRL_PARITY_ODD\n") : printf("CTRL_PARITY_EVEN\n");
        #else
        printf("status: %i\n",channels[i].status);
        printf("flag A: %i\n",channels[i].flag_a);
        printf("flag B: %i\n",channels[i].flag_b);
        printf("flag C: %i\n",channels[i].flag_c);
        printf("parity: %i\n",channels[i].parity);
        #endif

        printf("\n");
    }
}

int main(void)
{
	control_channel_t channels[8];
    read_channels((void*)&input_data, (void*)&channels);
    print_output(&channels);
}