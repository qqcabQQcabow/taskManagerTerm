#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> // usleep()
#include <termios.h> // для getch()

#define MAX_LEN_TASK 1000
#define MAX_TASK_ON_DAY 100
#define MAX_DATE_LEN 100
#define IN_WORK 0
#define DONE 1
#define DEAD 2


char getch() { // the best func in world!!!
    char buf = 0;
    struct termios old = { 0 };
    fflush(stdout);
    if (tcgetattr(0, &old) < 0) perror("tcsetattr()");
    old.c_lflag    &= ~ICANON;   // local modes = Non Canonical mode
    old.c_lflag    &= ~ECHO;     // local modes = Disable echo. 
    old.c_cc[VMIN]  = 1;         // control chars (MIN value) = 1
    old.c_cc[VTIME] = 0;         // control chars (TIME value) = 0 (No time)
    if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0) perror("read()");
    old.c_lflag    |= ICANON;    // local modes = Canonical mode
    old.c_lflag    |= ECHO;      // local modes = Enable echo. 
    if (tcsetattr(0, TCSADRAIN, &old) < 0) perror ("tcsetattr ~ICANON");
    return buf;
 }


typedef struct zadacha{
	char text_task[MAX_LEN_TASK];
	int status; // 0 - в процессе, 1 - сделано, 2 - просрочено
}task;

typedef struct dayday{
	time_t date;
	struct zadacha task_list[MAX_TASK_ON_DAY];
	int task_list_size;
}day;

time_t convertToUnixTime(const char* input_date) {
    struct tm timeinfo = {0};
    int day, month, year;

    // Разбираем строку на составляющие
    sscanf(input_date, "%d %d %d", &day, &month, &year);

    // Корректируем значения
    timeinfo.tm_mday = day;
    timeinfo.tm_mon = month - 1;  // Месяцы начинаются с 0
    timeinfo.tm_year = (year < 100) ? year + 2000 - 1900 : year - 1900;  // Коррекция года в зависимости от формата YY или YYYY

    // Преобразуем в Unix-время
    return mktime(&timeinfo);
}

time_t str_date_to_unix_time(char* input){  
	time_t unix_time = convertToUnixTime(input);
	return unix_time;
}

// сделать 1 задачу из строки(возвращает структуру с этой строкой и статусом)
task create_task(char* s, int status){ 
	task new; 
	new.status = status;
	strcpy(new.text_task, s);
	return new;

}

// создаёт день со списком задач(возвращает структуру с массивом из задач, его размером и датой)
day create_day(time_t date, task* task_list, int len_task_list){ 
	day new; 
	for (int i = 0; i < len_task_list; i++){
		new.task_list[i] = task_list[i];
	}
	new.date = date;
	new.task_list_size = len_task_list;
	return new;
}

// добавляет созданный день к списку дней
int append_day(day** day_list, int *len_day_list, day new_day){ 
	(*len_day_list)++;
	(*day_list) = (day*)realloc(*day_list, sizeof(day)*(*len_day_list));
	(*day_list)[*len_day_list-1] = new_day;
	return 0;
}

// ввод даты и её проверка на корректность, если всё ок, то вернет 3, иначе 1
int enter_date(char* input){ 
	fgets(input, MAX_DATE_LEN, stdin);
	int fl, day, month, year;
	
	input[strlen(input)-1]='\0';
	if((sscanf(input, "%d %d %d", &day, &month, &year)) != 3) return 0;
	if((day > 31) || (month > 12) || (year > 99)) return 0;
	return 1;

}

// смена статуса у определённой задачи
int change_status(task* taska, int new_status){ 
	taska->status = new_status;
	return 0;
}

// смена определённой задачи на s(с сохранением статуса)
int change_task(task* taska, char* s){ 
	strcpy(taska->text_task, s);
	return 0;
}

// удаление задачи из 1 дня
int remove_task(day* day_one, char* c, int* len_task_list){
	for (int i = 0; i < *len_task_list; i++){
		if(!strcmp(day_one->task_list[i].text_task, c)){
			for (int j = i; j < *len_task_list-1; j++)
			{
				day_one->task_list[j] = day_one->task_list[j+1];
			}
			(*len_task_list)--;
			return 0;
		}
	}
	return 1;
}

// добавить задачу ко дню
int append_task(day* dayd, task taska){ 
	dayd->task_list_size++;
	dayd->task_list[dayd->task_list_size-1] = taska;
	return 0;
}

// удаляет день из списка дней
int remove_day(day** day_list, time_t date, int *len_day_list){ 
	for(int i =0; i < *len_day_list; i++){
		if((*day_list)[i].date == date){
			(*day_list)[i] = (*day_list)[*len_day_list-1];
			(*len_day_list)--;
			return 0;
		}
	}
	return 1;
}

// разбивает структуру задачи на задачу и статус
void give_task_from_file(char* s, int* status){
	*status = s[strlen(s)-1] -48;
	if((s[strlen(s)-1] >= 48) && (s[strlen(s)-1] <= 57)){
		s[strlen(s)-2] = '\0';
	}
}

// DD MM YY -> unix
time_t str_to_num(char* s) { // Определение функции с аргументом s, который представляет строку
    time_t res = 0; // Переменная для хранения результата

    for (int i = 0; i < strlen(s); i++) { // Цикл, проходящий по символам строки
        if ((s[i] >= 48) && (s[i] <= 57)) { // Если символ является цифрой (ASCII-коды 48-57)
            res = res * 10 + ((int)s[i] - 48); // Постепенно собираем число по разрядам, умножая на 10 и прибавляя новую цифру
        }
    }

    return res; // Возвращаем число
}

// unix -> DD MM YY
char* unixTimeToString(time_t unixTime) {
    struct tm *timeinfo;
    timeinfo = localtime(&unixTime);

    // Выделяем память под строку для времени
    char *buffer = (char*)malloc(9 * sizeof(char)); // "DD MM YY\0" требует 9 символов

    // Форматируем строку согласно вашим требованиям "DD MM YY"
    strftime(buffer, 9, "%d %m %y", timeinfo);

    return buffer; // Возвращаем строку с временем
}

// вывод списка задач
void print_days(day* days, int days_len){
	for (int i = 0; i < days_len; i++){
		printf("date: %s\n", unixTimeToString(days[i].date));
		for (int j = 0; j < days[i].task_list_size; j++){
			printf("	%d. %s %d\n",j+1, days[i].task_list[j].text_task, days[i].task_list[j].status);
		}
	}
}

// запись списка задач в файл
void write_days_in_file(day* days, int days_len, FILE* f){
	for (int i = 0; i < days_len; i++){
		fprintf(f,"%ld\n", days[i].date);
		for (int j = 0; j < days[i].task_list_size; j++){
			fprintf(f,"%s %d\n", days[i].task_list[j].text_task, days[i].task_list[j].status);
		}
		fprintf(f, "\n");
	}

}

// сортировка дней bubble-sotr'ом
void sort_days(day** days, int days_len){
	day temp;
	for (int i = 0; i < days_len; i++){
		for (int j = i+1; j < days_len; j++){
			if((*days)[i].date > (*days)[j].date){
				temp = (*days)[j];
				(*days)[j] = (*days)[i];
				(*days)[i] = temp;
			}
		}
	}
}

// возвращает список дней из файла
day* give_data_from_file(FILE* f, int* days_len){
	char inp[MAX_LEN_TASK];
	char inp_date[MAX_DATE_LEN];

	day* days = NULL;

	int s_size = 0;
	task* s;
	
	int status, date;
	while(fgets(inp_date, MAX_DATE_LEN, f) != NULL){
		s_size = 0;
		s = (task*)malloc((s_size+1)*sizeof(task));
		fgets(inp, MAX_LEN_TASK, f);
		inp[strlen(inp)-1] = '\0';
		give_task_from_file(inp, &status);
		while(inp[0] != '\0'){
			s_size++;
			if(s_size == MAX_TASK_ON_DAY) return 0;
			s = realloc(s, s_size*sizeof(task));
			s[s_size-1] = create_task(inp, status);
			fgets(inp, MAX_LEN_TASK, f);
			inp[strlen(inp)-1] = '\0';
			give_task_from_file(inp, &status);
		}
		date = str_to_num(inp_date); 
		if(s_size != 0){
			append_day(&days, days_len, create_day(date, s, s_size));
		}
		s=NULL;
	}
	return days;
}


int main(){
	
	// размер массива дней, есть ли день в массиве, индекс нужного дня, статус выполнения задачи 
	int days_len=0, d_flag=0, day_index, status;
	
	// массив дней
	day* days = (day*)malloc(sizeof(day)*(days_len-1));
	
	// дата
	time_t date;
	
	// выбор пользователя
	char change;
	
	// номер события, вводимый статус
	int task_num, new_status;
	
	// флаг для главного цикла программы
	int inupt_flag=1;
	
	//массив с задачами и его размер
	int s_size = 0;
	task* s;
	
	// вводимые задачи
	char inp[MAX_LEN_TASK];
	
	// вводимая дата
	char inp_date[MAX_DATE_LEN];
	// переход в домашний каталог
	chdir(getenv("HOME"));
	int file_flag;
	FILE* f = fopen(".task_manager.txt", "r");
	if(f == NULL){
		file_flag = 1;
	}
	else{
		days = give_data_from_file(f, &days_len);
		fclose(f);
	}	
	printf("Welcome!\n");
	while(inupt_flag){
		// если программу запустили первый раз
		if(file_flag){
			printf("1. Make task\n2. Exit\n");
			change = getch();
			switch(change){
				case('1'):
					// открытие файла для записи
					f = fopen(".task_manager.txt", "w");
					// ввод списка задач из stdin
					printf("Enter date in format: DD MM YY, press enter to end:\n >>> ");
					while(enter_date(inp_date)){
						s_size = 0;
						s = (task*)malloc((s_size+1)*sizeof(task));
						fgets(inp, MAX_LEN_TASK, stdin);
						inp[strlen(inp)-1] = '\0';
						while(inp[0] != '\0'){
							s_size++;
							s = realloc(s, s_size*sizeof(task));
							s[s_size-1] = create_task(inp, IN_WORK);
							fgets(inp, MAX_LEN_TASK, stdin);
							inp[strlen(inp)-1] = '\0';
						}
						date = str_date_to_unix_time(inp_date);
						append_day(&days, &days_len, create_day(date, s, s_size));
						s=NULL;
                        printf("Enter date in format: DD MM YY, press enter to end:\n >>> ");
					}
					// сортировка дней
					sort_days(&days, days_len);
					// запись массива в файл
					write_days_in_file(days, days_len, f);
					file_flag = 0;
					fclose(f);
					f = NULL;
					break;
				case('2'):
					if(f != NULL) fclose(f);
					return 0;
				default:
					inupt_flag = 1;
			}

		}
		// если программу ранее запускали
		else{
			printf("\n1.Make tasks.\n"
				"2.Change task status.\n"
				"3.Change task.\n"
				"4.Remove task.\n"
				"5.Remove day.\n"
				"6.View tasks.\n"
				"7.Calendar\n"
				"8.Exit.\n");
			change = getch();

			switch(change){
				case('1'):
					system("clear");
					printf("Enter date in format: DD MM YY, press enter to end:\n >>> ");

					// запись данных из stdin в стуктуру
					while(enter_date(inp_date)){
						d_flag = 1;
						s_size = 0;
						s = (task*)malloc((s_size+1)*sizeof(task));
                        printf("Enter tasks, press enter to end\n >>> ");
						fgets(inp, MAX_LEN_TASK, stdin);
						inp[strlen(inp)-1] = '\0';
						while(inp[0] != '\0'){
							s_size++;
							s = realloc(s, s_size*sizeof(task));
							s[s_size-1] = create_task(inp, IN_WORK);
							printf(" >> ");
							fgets(inp, MAX_LEN_TASK, stdin);
							inp[strlen(inp)-1] = '\0';
						}
						date = str_date_to_unix_time(inp_date);
						// поиск введёной даты в массиве
						for (int i = 0; i < days_len; ++i){
							if(days[i].date == date){
								d_flag = 0;
								day_index = i;
								break;
							}
						}
						//если даты нет, то делаем день
						if(d_flag) append_day(&days, &days_len, create_day(date, s, s_size));
						// если дата уже есть, то список задач цепляем к ней	
						else{
							for (int i = 0; i < s_size; ++i){
								append_task(&days[day_index], s[i]);
							}
						}
						s=NULL;
                        printf("Enter date in format: DD MM YY, press enter to end:\n >>> ");
					}
					// сортировка дней по возрастанию O(n^2)
					sort_days(&days, days_len);
					break;
				case('2'):
					system("clear");
					// вывод массива
					print_days(days, days_len);
					// ввод дня
					printf("Select day:\n>>> ");
					enter_date(inp_date);
					date = str_date_to_unix_time(inp_date);
					// поиск этого дня
					for (int i = 0; i < days_len; i++){
						if(days[i].date == date){
							d_flag = 1;
							// вывод событий этого дня
							system("clear");
							printf("date: %s\n", unixTimeToString(days[i].date));
							for (int j = 0; j < days[i].task_list_size; j++){
								printf("	%d. %s %d\n", j+1, days[i].task_list[j].text_task, days[i].task_list[j].status);
							}
							// ввод и изменение статуса
							printf("Select task:\n>> ");
							scanf("%d", &task_num);
							printf("Enter status:\n>>> ");
							scanf("%d", &new_status);
							getchar();
							change_status(&days[i].task_list[task_num-1], new_status);
						}
					}
					if(!d_flag) printf("This day is not exist.\n");
					break;
				case('3'):
					system("clear");
					// вывод данных
					print_days(days, days_len);
					// ввод события
					//printf("Выберите день, события которого вы хотите изменить, enter для пропуска:\n >>> ");
					printf("Select day for change task, press enter fo continue:\n >>> ");
					enter_date(inp_date);
					date = str_date_to_unix_time(inp_date);
					for (int i = 0; i < days_len; i++){
						if(days[i].date == date){
							d_flag = 1;
							system("clear");
							// вывод событий этого дня
							printf("date: %s\n", unixTimeToString(days[i].date));
							for (int j = 0; j < days[i].task_list_size; j++){
								printf("	%d. %s %d\n", j+1, days[i].task_list[j].text_task, days[i].task_list[j].status);
							}
							printf("Select task for change: ");
							scanf("%d", &task_num);
							getchar();
							if(task_num > days[i].task_list_size){
								printf("Task is not exist.\n");
								break;
							}
							// изменение задания
							printf("Enter new task:\n >>> ");
							fgets(inp, MAX_LEN_TASK, stdin);
							inp[strlen(inp)-1] = '\0';
							change_task(&days[i].task_list[task_num-1], inp);
						}
					}
					if(!d_flag){
						printf("Day is not exist.\n");
					}
					break;
				case('4'):
					system("clear");
					// вывели данные из массива
					print_days(days, days_len);
					// Ввод даты дня
					printf("Select day for remove task:\n >>> ");
					enter_date(inp_date);
					date = str_date_to_unix_time(inp_date);
					// удаление события
					for(int i =0; i < days_len; i++){
						if(days[i].date == date){
							d_flag = 1;
							// вывод событий этого дня
							printf("date: %s\n", unixTimeToString(days[i].date));
							for (int j = 0; j < days[i].task_list_size; j++){
								printf("	%d. %s %d\n", j+1, days[i].task_list[j].text_task, days[i].task_list[j].status);
							}
							// ввод события для удаления
                            printf("Select task for remove: ");
							scanf("%d", &task_num);
							getchar();
							remove_task(&days[i],days[i].task_list[task_num-1].text_task, &days[i].task_list_size);
						}
					}
					if(!d_flag){
						printf("Day is not exist.\n");
					}
					break;
				case('5'):
					system("clear");
					// вывод данных
					print_days(days, days_len);
					if(days_len == 0){
						printf("Empty!\n");
					}
					// выбор дня для удаления
					printf("Select day for remove:\n >>> ");
					enter_date(inp_date);
					date = str_date_to_unix_time(inp_date);
					d_flag = remove_day(&days, date, &days_len);

					if(d_flag){
						printf("Day is not exist.\n");
					}
					else{
						printf("Good!\n");
					}
					break;
				case('6'):
					system("clear");
					// вывод данных
					print_days(days, days_len);
					if(days_len == 0){
						printf("Empty!\n");
					}
					break;
				case('7'):
					system("clear");
					system("cal");
					break;
				case('8'):
					// запись данных в файл при закрытии приложения
					f = fopen(".task_manager.txt", "w");
					write_days_in_file(days, days_len, f);
					fclose(f);
					return 0;
				default:
					system("clear");
			}

		}

	}
	return 0;

}
