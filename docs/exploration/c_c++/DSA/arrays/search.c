/*
* Exploration of advanced search algorithms in C
* 1. Binary search 
*/

#include <stdio.h> 

int binary_search(int array[10],int length, int search_element)
{
    /*
    Assume binary search as dividing array space in the power of twos
    Each division operation then becomes reducing the exponent of 2 by 1.
    
    | x1 | x2 | x3 | x4 | x5 | x6 | x7 | x8 | x9 | x10 | x11 | x12 | x13 | x14 | x15 | x16 | 
    | 2^0| 2^1|    | 2^2|    |    |    | 2^3|    |     |     |     |     |     |     | 2^4 |
    
    - Division by two in each step is essentially shrinking the space by 2^(n-1)
    
    This is the exact logic behind fibonacci.
    Instead of considering power of 2, fibonacci search consider power of golden ratio
    NOTE: 
    - Although this algorithm has computational complexity log_2(N), it is less preferred in modern CPUs
    - Big jumps of index happens because of large block sizes; Entire search space is shrink by a factor of two every time
    - This hurts the caching
    - Hence, algorithms with higher block sizes are preferred; Refer block search
    */

    int left=0, right=length-1, middle =0;
    while (left <= right){
        middle = left + (right-left)/2; // Prevents overflow. (right + left) operation may overflow int datatype.  
        
        if (array[middle] == search_element) 
        {            
            return middle;
        }
        else if (array[middle] > search_element)
        {
            right = middle-1;
        }
        else if (array[middle] < search_element){
            left = middle+1;   
        }
        
    }
    return -1;
}


int fibonacci_search(int search_item, int *array, int length)
{
    /*
        |<----------------------------- Current Window: Size F_k1 ---------------------------->|
        [ offset ] |<--- Left Piece: Size F_{k1-2} --->| i |<--- Right Piece: Size F_{k1-1} --->|
                                            |
                                            v
        |<-------------------------------- Current Window: Size F_k2 = F_{k1-2} -------------------------------------------->|
        [ offset ] |<--- Left Piece: Size F_{k1-4} = F_{k2} - F{k1-2} --->| i |<--- Right Piece: Size F_{k2-3} = F{k1-2} --->|

    */

    int fi_k2 = 0;
    int fi_k1 = 1;
    int fi_k = fi_k1 + fi_k2;

    // find smallest fibo number less than or equal to length
    while(fi_k <= length){
        fi_k2 = fi_k1;
        fi_k1 = fi_k;
        fi_k = fi_k1+fi_k2;
    }

    // mask eliminated range from left hand side of array
    int offset = -1;

    while(fi_k>1){
        int i = min(offset + fi_k2, length-1)
        if(search_item > array[i]){
            offset = i;
            fi_k = fi_k1;
            fi_k1 = fi_k2;
            fi_k2 = fi_k - fi_k1;
        }
        if(search_item < array[i]){
            fi_k = fi_k2;
            fi_k1 = fi_k - fi_k2;
            fi_k2 = fi_k - fi_k1;
        }
        else 
            return i;
    }
}

int block_search(int search_item, int *array, int length){

}

int main(){
    int list[] = {1,2,8,22,99,238,242,4789};
    int length = sizeof(list) / sizeof(int);
    int search_element = 213;
    int index = binary_search(list, length, search_element);
    printf("Element found at index %d is %d", index, search_element);
}

