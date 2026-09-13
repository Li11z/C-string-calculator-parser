#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/*
parse a string formula to be mathematically viable and executable

1. user input valid math string
2. system goes char by char
    -> if space or tab we delete -> e.g a + b becomes a+b, 3.14    (90 -    2) -> 3.4(90-2)
    -> if number we put it in a string number until we reach the end since num can easily be single degit: 1, or multi digit 11
    -> once done we convert that string to a int
    -> event it is double/float and has decimal we need to also keep track of '.' and insert it in the array e.g 3.14
    -> need to establish a arithmetic logic i.e PEDMAS
        -> system must detect paranteses and prioritise content inside it
        -> expoentials -> division -> mult -> substraction <-> addition
        -> key tokes: '(), +,-,/,*, ^' must also differentiate between - for negative num, or - as operation
        -> soliution  keep track of next char , - followed by number means it sjust a negative number
        -> -followed by ( transforms to -1* for simplicity
        -> similarly if a number is followed by paranteses its rewritten as mult e.g 3.14(5) -> 3.14*(5)

    -> error handling
        -> no duplicates (2 or more): e.g ++,--,//,^^ ,..,  exception: ** (unless 3 or more e.g ***) to be interprted as ^ and (( explained below
        -> number has more than one decimal e.g 3.14.54 this is incorect we must keep count that if . appears more than once in a number its flagged
        -> none accepted tokens i.e if not +-*./()^ or a digit then flagged as error illegal char

    first we need a way to handle parantheses (), espcailly inbedded ones
    - we keep tracker position of each parantheses, e.g a-(b*(d-e))/(23-3)
    we keep track of first paranteses position at pos:3, then we know new open paranteses appears at pos 6
    so pos6 is the newer instance and we know thus that the closing paranteses at pos 10 is for pos6
    we thus execute operation from 6+1 to 10-1 first , hwoever we still know that we have a paranteses that starts at pos3,
    instead of starting all the way back at 0, we just restart from pos3
    similarly if parnateses are over we just restart from the parent position e.g 3
example :  a-(b*(d-e))/(f+g)
a-1*(b*(d-e))/(f+g)
-> ( at 3 -> ( found ar 6 -> ) found at 10 => execute 6+1 to 10-1 = d-e = x1
a-(b*x1)/(f+g)
-> return to ( at 3 -> found ) at now 8 -> execute 3+1 to 8-1 => b*x1 = x2
a-x2/(f+g)
-> return at 3 -> (found at 6 -> ) found at 10 -> execute 6+1 to 10-1 -> f+g = x3
a-x2/x3
-> continue at 3 -> reach end of instruction no more () found !

better yet we compute the size of the new expression and add it to counter e.g we know that x1 is 2 char and original ( was at 6
so we know to move current index from 6 to 5+2 so index is 7 (we do -1 just for safety but theoretically 6+2 should work i think)
-> now simply follow order of first to execute ^->/->*->+<->-
*/

void my_getline(char **line, int *size) {
    *line = malloc((sizeof(char) * 100) + 1);
    *size = 0;
    int memory_space = 100;

    int c = fgetc(stdin);

    while (c != '\n' && c != EOF) {
        (*line)[*size] = (char) c;
        (*size)++;

        if (*size + 1 >= memory_space) {
            memory_space *= 2;

            char *temp = realloc(*line, sizeof(char) * memory_space);

            if (temp == NULL) {
                // in case of failure we terminate gracefuly
                printf("realloc failed\n");
                free(*line);
            }
            *line = temp;
        }

        c = fgetc(stdin);
    }
    (*line)[*size] = '\0';
};

bool string_concat(char **line, int *size) {
    int *pos_space = malloc((sizeof(int) * *size) + 1);
    int num_spaces = 0;

    char *input_buffer = malloc((sizeof(char) * *size) + 1);
    int j = 0;

    for (int i = 0; i < *size; i++) {
        if (isspace((unsigned char) ((*line)[i]))) {
            continue;
        } else {
            input_buffer[j] = (*line)[i];
            j++;
        }
    }
    input_buffer[j] = '\0';

    *line = input_buffer;
    *size = j;

    printf("input space removed:\n");
    for (int i = 0; i < *size; i++) {
        printf("%c", (*line)[i]);
    }


    return true;
}

void string_enrich(char **line, int *size) {
    /*
     conditions: a( -> a*(
                -( -> -1*(
                ** -> ^
                x-y -> x+-y

     */

    char *input_buffer = malloc((sizeof(char) * *size * 4) + 1);
    //since worse case scenario our buffer can become 4times as big (correct technique is to use realloc but im lazy)
    int buffer_size = 0;

    for (int i = 0; i < *size; i++) {
        if (isdigit((unsigned char) (*line)[i]) && (*line)[i + 1] == '(') {
            //printf("\n%c %c" , (*line)[i] ,(*line)[i+1] );

            input_buffer[buffer_size] = (*line)[i];
            buffer_size++;
            input_buffer[buffer_size] = '*';
            buffer_size++;
            //input_buffer[buffer_size] = (*line)[i + 1];
            //buffer_size++;
            //i++;
        } else if ((*line)[i] == '-' && (*line)[i + 1] == '(') {
            input_buffer[buffer_size] = '+';
            buffer_size++;

            input_buffer[buffer_size] = (*line)[i];
            buffer_size++;

            input_buffer[buffer_size] = '1';
            buffer_size++;

            input_buffer[buffer_size] = '*';
            buffer_size++;

            //input_buffer[buffer_size] = (*line)[i + 1];
            //buffer_size++;
            //i++;
        } else if ((*line)[i] == '*' && (*line)[i + 1] == '*') {
            input_buffer[buffer_size] = '^';
            buffer_size++;

            i++;
        } else if ((*line)[i] == '-' && isdigit((unsigned char) (*line)[i + 1]) && isdigit(
                       (unsigned char) (*line)[i - 1]) && i > 0) {
            input_buffer[buffer_size] = '+';
            buffer_size++;

            input_buffer[buffer_size] = (*line)[i];
            buffer_size++;

            //input_buffer[buffer_size] = (*line)[i + 1];
            //buffer_size++;

            //i++;
        } else {
            input_buffer[buffer_size] = (*line)[i];
            buffer_size++;
        }
    }
    input_buffer[buffer_size] = '\0';

    *line = input_buffer;
    *size = buffer_size;

    printf("\n buffer content: \n");
    for (int i = 0; i < *size; i++) {
        printf("%c", (*line)[i]);
    }
}

void error_finding(char **line, int *size) {
    for (int i = 0; i < *size; i++) {
        //if iis not accepted token i.e operation, paranteses, or digits, or dot
        //if is duplicate operation
        //if . appears twice in one number sequence
        //if . appears infront of a number, if number ends with .
        //make sure every ( has an ),
        // make sure that every ) has an (
        // back to back operation e.g +*, *+, -/, , etc... we shoudlnt have operators next tp each other
        // operation starting with operator (except -)
    }
}

double simple_operation(double left_num, double right_num, char operation) {

    double x=0;

    if (operation == '+') {
        x=left_num + right_num;
    }
    else if (operation == '*') {
        x=left_num * right_num;
    }
    else if (operation == '/') {
        x=left_num / right_num;
    }
    else if (operation == '^') {
        x=pow(left_num,right_num);
    }


    return x;
}

void parser(char **line, int *size) {
    char *operation_stack = malloc((sizeof(char) * *size) + 1);
    int top_operation_stack = -1;

    double *number_stack = malloc((sizeof(double) * *size) + 1);
    int top_number_stack = -1;

    for (int i = 0; i < *size; i++) {

        if (isdigit((unsigned char) (*line)[i]) || (*line)[i] == '-') {
            int j = i;
            int k = 0;
            char *number_buffer = malloc((sizeof(char) * *size) + 1);

            while (isdigit((unsigned char) (*line)[j]) || (*line)[j] == '.' || (*line)[j] == '-') {
                number_buffer[k] = (*line)[j];
                j++;
                k++;
            }
            number_buffer[k] = '\0';
            double number = atof(number_buffer);
            i = j - 1;
            number_stack[top_number_stack + 1] = number;
            top_number_stack++;
        }

        else if ((*line)[i] == '+' || (*line)[i] == '*' || (*line)[i] == '/' || (*line)[i] == '^' || (*line)[i] =='(') {
            printf("\n!!!!!!HELLO0!\n");
            if (top_operation_stack >= 0) {
                printf("\n!!!!!!HELLO1!\n");
                char op = operation_stack[top_operation_stack];

                if (((*line)[i] == '+' && (op == '*' || op == '/' || op == '^')) || (((*line)[i] == '*' || (*line)[i] == '/') && op == '^')) {
                    printf("\n!!!!!!HELLO2!\n");
                    //simulating popping even tho unecesary this will become a genral function later on so keeping convention alive
                    char pop_buffer = operation_stack[top_operation_stack];
                    operation_stack[top_operation_stack] = '\0';
                    top_operation_stack--;

                    operation_stack[top_operation_stack + 1] = (*line)[i];
                    top_operation_stack++;
                    operation_stack[top_operation_stack + 1] = pop_buffer;
                    top_operation_stack++;


                    if (isdigit((unsigned char) (*line)[i+1]) || (*line)[i+1] == '-') {
                        int j = i+1;
                        int k = 0;
                        char *number_buffer = malloc((sizeof(char) * *size) + 1);

                        while (isdigit((unsigned char) (*line)[j]) || (*line)[j] == '.' || (*line)[j] == '-') {
                            number_buffer[k] = (*line)[j];
                            j++;
                            k++;
                        }
                        number_buffer[k] = '\0';
                        double number = atof(number_buffer);

                        double num_pop_buffer_1 = number_stack[top_number_stack];
                        top_number_stack--;

                        double num_pop_buffer_2 = number_stack[top_number_stack];
                        top_number_stack--;

                        number_stack[top_number_stack + 1] = number;
                        top_number_stack++;

                        number_stack[top_number_stack + 1] = num_pop_buffer_2;
                        top_number_stack++;

                        number_stack[top_number_stack + 1] = num_pop_buffer_1;
                        top_number_stack++;

                        i = j - 1;
                    }
                }
                else {
                    operation_stack[top_operation_stack + 1] = (*line)[i];
                    top_operation_stack++;
                }

            }
            else {
                operation_stack[top_operation_stack + 1] = (*line)[i];
                top_operation_stack++;
            }

        }

        else if ((*line)[i] == ')' || (*line)[i] == '\0') {
            // pop operation 1= op
            // pop num 1 =x
            // pop num 2 =y
            // call simple z=operation(x,y,op)
            //push z into num stack
            //repeat until we pop ( or op stacks is empty (num stack should have only the final result left)
            //dont forget to make this function into a double return value when over
        }
    }


    printf("\n number stack content: \n");
    for (int i = 0; i < top_number_stack + 1; i++) {
        printf("%lf; ", number_stack[i]);
    }

    printf("\n operation stack content: \n");
    for (int i = 0; i < top_operation_stack + 1; i++) {
        printf("%c", operation_stack[i]);
    }
}

/*
1. go char by char
2. envounter ( -> store position
3. encounter another (-> add new possize to stack
4. encounter ) -> get pos, pop stack for latest pos => we end up with range
5. solve content -> get result  => we need to store this result and its position somehow
6. continue progressing ->
 */

int main() {
    char *line = NULL;
    int size = 0;

    printf("write your math string:\n");

    my_getline(&line, &size);
    string_concat(&line, &size);
    string_enrich(&line, &size);
    //error_finding(&line, &size);

    parser(&line, &size);

    //printf("\nthis is new size: %d", size);

    return 0;
}
