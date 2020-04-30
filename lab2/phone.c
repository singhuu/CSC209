#include <stdio.h>

int main(){

char phone[11];
int val;
scanf("%s %d", phone, &val);
if(val == -1){
printf("%s\n", phone);
return 0;
}
else if(val >= 0 && val <= 9){
printf("%c\n", phone[val]);
return 0;
}
else {
printf("ERROR\n");
return 1;
}
}
