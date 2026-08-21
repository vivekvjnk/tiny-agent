

int bubble_sort(int *array, int length){
    for(int i=0;i<length;i++){
        for(int j=0;j<length-1-i;j++){
            if(array[j]>array[j+1]){
                int tmp = array[j];
                array[j] = array[j+1];
                array[j+1] = tmp;
            }
        }
    }
}
int exchange_sort(int *array,int length){
    for(int i=0;i<length;i++){
        for(int j=i;j<length-1;j++){
            if(array[i]>array[j]){
                int tmp = array[j];
                array[j]= array[i];
                array[i] = tmp;
            }
        }
    }
}

void selectionSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (arr[j] < arr[min_idx]) min_idx = j;
        }
        // Swap
        int temp = arr[min_idx];
        arr[min_idx] = arr[i];
        arr[i] = temp;
    }
}

int main(){
    // int array[] = {3,2,22,89,21,11,19};
    // int length=7;
    
}