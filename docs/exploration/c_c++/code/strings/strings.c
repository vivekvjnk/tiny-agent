//1. Simple string comparison
#include <stdio.h>
int string_compare(const char *s1, const char *s2, int len_s1, int len_s2){
    if(len_s1 != len_s2)
        return 0;
    
    for(int i=0; i< len_s1; i++){
        if(s1[i] != s2[i])
        {
            return 0;
        }
    }
    return 1;
}

int main(){
    const char s1[]  = "Hello";
    const char s2[] = "Hello";
    int result = string_compare(s1,s2, sizeof(s1)/sizeof(char), sizeof(s2)/sizeof(char));
    printf("Result %d\n",result);
}