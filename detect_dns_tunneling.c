#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include <stdio.h>      // for debugging
#include <math.h>       // for debugging

#ifdef BASE_N_ENCODE
    #define ND_base_x 3.8
    #define ND_base_s 0.5
#elif HEX_ENCODE
    #define ND_base_x 2.6
    #define ND_base_s 0.265
#endif

#define len_arr 350000

double score_arr[len_arr];
int subscore_arr[len_arr];

// aplhabet frequency table
// http://www7.plala.or.jp/dvorakjp/hinshutu.htm
double frequency[] = {
8.446499792,
1.644278059,
3.195576109,
3.870904598,
11.40962588,
1.984389146,
1.820450708,
4.255058847,
7.130098608,
0.205534757,
0.8955443,
3.87335144,
2.671951846,
6.770412782,
7.051799653,
2.023538623,
0.08319264,
6.222320096,
6.973500697,
8.184687661,
2.953338716,
0.998311679,
1.698108591,
0.176172649,
1.793535443,
0.078298955
};

void string_tolower(char* str)
{
    for(int i = 0; str[i]; i++){
        str[i] = tolower(str[i]);
    }
}

double score_word(char* word)
{
    int len_word = strlen(word);
    double score = 0.0;
    string_tolower(word);
    for(int i = 0; i < len_word; i++)
    {
        char c = word[i];
        if(isalpha(c))
        {
            score += frequency[c - 'a'];
        }
        else if(c == '0')
        {
            score += 3.2;
        }
        else if(c == '1')
        {
            score += 2.4;
        }        
        else if(isdigit(c))
        {
            score += 0.5;
        }
        else
        {
            score += 6.0;
        }
    }
    return score;
}

int count_num_x(char* fqdn, char x)
{
    int num_x = 0;
    for(int i = 0; i < (int) strlen(fqdn); i++)
    {
        if(fqdn[i] == x)
        {
            num_x++;
        }
    }
    return num_x;
}

int count_subdomain_length(char* fqdn)
{
    int num_dots = count_num_x(fqdn, '.');
    int count = 0;

    int i = 1;
    for (char *p = strtok(fqdn, "."); p != NULL; p = strtok(NULL, "."))
    {
        if(i == num_dots)
        {
            count--; // remove last '.'
            break;
        }
        count += strlen(p) + 1; // '+ 1' means counting '.'
        i++;
    }
    return count;
}

void get_subdomain(char* fqdn, char* subdomain)
{
    int last_2_dot_pos = 0;
    int last_dot_pos = 0;
    int len_fqdn = strlen(fqdn);
    for(int i = 0; i < len_fqdn; i++)
    {
        if(fqdn[i] == '.')
        {
            last_2_dot_pos = last_dot_pos;
            last_dot_pos = i;
        }
    }
    strncpy(subdomain, fqdn, len_fqdn);
    for(int i = last_2_dot_pos; i < len_fqdn; i++)
    {
        subdomain[i] = '\0';
    }
}

// @return true: detected, false: not detected
bool detection_filter(double score)
{
    return ND_base_x - 2 * ND_base_s <= score && score <= ND_base_x + 2 * ND_base_s;
}

// ======== [for module testing] =======

double ave(double* arr, long long int N)
{
    double v = 0.0;
    for(int i = 0; i < N; i++)
    {
        v += arr[i];
    }
    v /= (double) N;
    return v;
}

double var(double* arr, double ave, long long int N)
{
    double v = 0.0;
    for(int i = 0; i < N; i++)
    {
        v += (arr[i] - ave) * (arr[i] - ave);
    }
    v /= (double) N;
    return v;
}

void strip(char* text)
{
    int pos = strlen(text) - 1;
    if(text[pos] == '\n')
    {   
        text[pos] = '\0';
    }
}

FILE* classifier_log;
void init_classifier_log()
{
    classifier_log = fopen("./classifier_result.csv", "a");
    if(!classifier_log)
    {
        perror("open classifier_log error");
        exit(1);
    }
}

void add_classifier_log(int threshold, double ND_x, double ND_s, int TP, int TN, int FP, int FN)
{
    fprintf(classifier_log, "%d,%.2f,%.2f,%d,%d,%d,%d\n", threshold, ND_x, ND_s, TP, TN, FP, FN);
}

void multi_version_classifier(int N, double* score_arr, int* subscore_arr, bool is_clean)
{
    init_classifier_log();
    for(int i = -3; i <= 2; i++)
    {
        for(int j = -2; j <= 2; j++)
        {
            for(int k = -8; k <= 8; k++)
            {
                int threshold = 12 + 4 * i;
                double ND_x = ND_base_x + (double) j * 0.1; // average
                double ND_s = ND_base_s + (double) k * 0.05; // sigma
                int detection_count = 0;
                for(int l = 0; l < N; l++)
                {
                    if(subscore_arr[l] >= threshold)
                    {
                        if(ND_x - 2 * ND_s <= score_arr[l] && score_arr[l] <= ND_x + 2 * ND_s)
                        {
                            detection_count++;
                        }
                    }
                }
                int TP = 0, TN = 0, FP = 0, FN = 0;
                /*
                                真の結果
                                正  負
                予測結果     正  TP  FP
                            負  FN  TN
                */
                if(is_clean) // should not be detected ()
                {
                    FP = detection_count;       // number of wrong detections
                    TN = N - detection_count;   // number of collect detections
                }
                else
                {
                    TP = detection_count;       // number of collect detections
                    FN = N - detection_count;   // number of wrong detections
                }
                add_classifier_log(threshold, ND_x, ND_s, TP, TN, FP, FN);
            }
        }
    }
}

void usage(char* argv[])
{
    printf("usage: %s WORD_LIST_FILE IS_CLEAN(Y/N)\n", argv[0]);
    exit(1);
}

int main(int argc, char* argv[])
{
    if(argc <= 2)
    {
        usage(argv);
    }

#ifdef BASE_N_ENCODE
    puts("[*] Base-N encode mode");
#elif HEX_ENCODE
    puts("[*] hex encode mode");
#else
    #error "[!] no mode specified."
#endif    

    FILE* fd;
    char *file_name = argv[1];
    fd = fopen(file_name, "r");
    if(!fd)
    {
        perror("fopen error");
        usage(argv);
    }
    bool is_clean = true;
    if(argv[2][0] == 'N')
    {
        puts("[*] inputted data is malicious");
        is_clean = false;
    }
    else
    {
        puts("[*] inputted data is clean");
    }

    char buf[1024];
    char subdomain[1024];
    long long int i = 0;
    int num_detected = 0;
    int over_threshold_count = 0;
    while(fgets(buf, sizeof(buf), fd))
    {
        strip(buf);
        // printf("%d\t%s\n", i + 1, buf);
        get_subdomain(buf, subdomain);
        int len_word = strlen(subdomain);
        double score = (double) score_word(subdomain) / (double) len_word;
        int num_dot = count_num_x(subdomain, '.');
        int num_hyphen = count_num_x(subdomain, '-');
        if(strlen(subdomain) - num_dot - num_hyphen >= 8 /* && 
            (double) strlen(subdomain) / (double) num_dot >= 10.0 &&
            (double) strlen(subdomain) / (double) (num_dot + num_hyphen) >= 10.0*/
            )
        {
            over_threshold_count++;
            if(detection_filter(score))
            {
                printf("%s\tdetected: %s (len_word=%d, score=%f)\n", file_name, buf, len_word, score);
                num_detected++;
            }
        }
        subscore_arr[i] = strlen(subdomain) - num_dot - num_hyphen;
        score_arr[i] = score;
        i++;
        if(i == len_arr)
        {
            puts("[*] too many inputs. break");
            break;
        }
    }
    long long int N = i;
    printf("N = %lld\n", N);
    double x = ave(score_arr, N);
    printf("x = %.10f\n", x);
    double v = var(score_arr, x, N);
    printf("v = %.10f\n", v);
    double sigma = sqrt(v);
    printf("s = %.10f\n", sigma);
    printf("x - 2s = %.10f\n", x - 2 * sigma);
    printf("x + 2s = %.10f\n", x + 2 * sigma);
    printf("num_detected = %d (detection rate: %f)\n", num_detected, ((double) num_detected / (double) N));
    printf("over threshold subdomains: %d (rate: %f)\n", over_threshold_count, ((double) over_threshold_count) / (double) N);
    multi_version_classifier(N, score_arr, subscore_arr, is_clean);
}

/*
        count = 0
        vowels = 'aeiouy'
        text = text.lower()
        text = "".join(x for x in text if x not in exclude)

        if text is None:
            return 0
        elif len(text) == 0:
            return 0
        else:
            if text[0] in vowels:
                count += 1
            for index in range(1, len(text)):
                if text[index] in vowels and text[index-1] not in vowels:
                    count += 1
            if text.endswith('e'):
                count -= 1
            if text.endswith('le'):
                count += 1
            if count == 0:
                count += 1
            count = count - (0.1*count)
*/