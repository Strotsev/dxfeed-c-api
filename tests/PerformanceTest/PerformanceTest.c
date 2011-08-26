

#include "DXFeed.h"
#include "DXErrorCodes.h"
#include "Logger.h"
#include <time.h>

#include <Windows.h>
#include "signal.h"
#include <stdio.h>

const char dxfeed_host[] = "mddqa.in.devexperts.com:7400";

dxf_const_string_t dx_event_type_to_string (int event_type) {
	switch (event_type) {
	case DXF_ET_TRADE: return L"Trade";
	case DXF_ET_QUOTE: return L"Quote";
	case DXF_ET_SUMMARY: return L"Summary";
	case DXF_ET_PROFILE: return L"Profile";
	case DXF_ET_ORDER: return L"Order";
	case DXF_ET_TIME_AND_SALE: return L"Time&Sale";
	default: return L"";
	}
}

/* -------------------------------------------------------------------------- */
static bool is_listener_thread_terminated = false;
CRITICAL_SECTION listener_thread_guard;

bool is_thread_terminate() {
    bool res;
    EnterCriticalSection(&listener_thread_guard);
    res = is_listener_thread_terminated;
    LeaveCriticalSection(&listener_thread_guard);

    return res;
}

/* -------------------------------------------------------------------------- */

void on_reader_thread_terminate(const char* host, void* user_data) {
    EnterCriticalSection(&listener_thread_guard);
    is_listener_thread_terminated = true;
    LeaveCriticalSection(&listener_thread_guard);

    printf("\nTerminating listener thread, host: %s\n", host);
}

/* -------------------------------------------------------------------------- */
int quotes_counter = 0;
bool doPrint = false;
FILE *output_stream;

void listener (int event_type, dxf_const_string_t symbol_name, const dxf_event_data_t* data, int data_count, void* user_data) {
    dxf_int_t i = 0;

	++quotes_counter;

	if (!doPrint)
		return;

    wprintf(L"Event: %s Symbol: %s\n",dx_event_type_to_string(event_type), symbol_name);
	fwprintf(output_stream, L"Event: %s Symbol: %s\n",dx_event_type_to_string(event_type), symbol_name);

    if (event_type == DXF_ET_QUOTE) {
	    dxf_quote_t* quotes = (dxf_quote_t*)data;

	    for (; i < data_count; ++i) {
		    wprintf(L"bid time=%i, bid exchange code=%C, bid price=%f, bid size=%i; "
		            L"ask time=%i, ask exchange code=%C, ask price=%f, ask size=%i\n",
		            quotes[i].bid_time, quotes[i].bid_exchange_code, quotes[i].bid_price, quotes[i].bid_size,
		            quotes[i].ask_time, quotes[i].ask_exchange_code, quotes[i].ask_price, quotes[i].ask_size);
			fwprintf(output_stream,L"bid time=%i, bid exchange code=%C, bid price=%f, bid size=%i; "
		            L"ask time=%i, ask exchange code=%C, ask price=%f, ask size=%i\n",
		            quotes[i].bid_time, quotes[i].bid_exchange_code, quotes[i].bid_price, quotes[i].bid_size,
		            quotes[i].ask_time, quotes[i].ask_exchange_code, quotes[i].ask_price, quotes[i].ask_size);
		}
    }
    
    if (event_type == DXF_ET_ORDER){
	    dxf_order_t* orders = (dxf_order_t*)data;

	    for (; i < data_count; ++i) {
		    wprintf(L"index=%i, side=%i, level=%i, time=%i, exchange code=%C, market maker=%s, price=%f, size=%i\n",
		            orders[i].index, orders[i].side, orders[i].level, orders[i].time,
		            orders[i].exchange_code, orders[i].market_maker, orders[i].price, orders[i].size);
			fwprintf(output_stream,L"index=%i, side=%i, level=%i, time=%i, exchange code=%C, market maker=%s, price=%f, size=%i\n",
		            orders[i].index, orders[i].side, orders[i].level, orders[i].time,
		            orders[i].exchange_code, orders[i].market_maker, orders[i].price, orders[i].size);
		}
    }
    
    if (event_type == DXF_ET_TRADE) {
	    dxf_trade_t* trades = (dx_trade_t*)data;

	    for (; i < data_count; ++i) {
		    wprintf(L"time=%I64i, exchange code=%c, price=%f, size=%I64i, day volume=%f\n",
		            trades[i].time, trades[i].exchange_code, trades[i].price, trades[i].size, trades[i].day_volume);
			fwprintf(output_stream,L"time=%I64i, exchange code=%C, price=%f, size=%I64i, day volume=%f\n",
		            trades[i].time, trades[i].exchange_code, trades[i].price, trades[i].size, trades[i].day_volume);
			
		}
    }
    
    if (event_type == DXF_ET_SUMMARY) {
	    dxf_summary_t* s = (dxf_summary_t*)data;

	    for (; i < data_count; ++i) {
		    wprintf(L"day high price=%f, day low price=%f, day open price=%f, prev day close price=%f, open interest=%i\n",
		            s[i].day_high_price, s[i].day_low_price, s[i].day_open_price, s[i].prev_day_close_price, s[i].open_interest);
		    fwprintf(output_stream,L"day high price=%f, day low price=%f, day open price=%f, prev day close price=%f, open interest=%i\n",
		            s[i].day_high_price, s[i].day_low_price, s[i].day_open_price, s[i].prev_day_close_price, s[i].open_interest);

		}
    }
    
    if (event_type == DXF_ET_PROFILE) {
	    dxf_profile_t* p = (dxf_profile_t*)data;

	    for (; i < data_count ; ++i) {
		    wprintf(L"Description=%s\n",
				    p[i].description);
			fwprintf(output_stream,L"Description=%s\n",
				    p[i].description);
	    }
    }
    
    if (event_type == DXF_ET_TIME_AND_SALE) {
        dxf_time_and_sale_t* tns = (dxf_time_and_sale_t*)data;

        for (; i < data_count ; ++i) {
            wprintf(L"event id=%ld, time=%ld, exchange code=%c, price=%f, size=%li, bid price=%f, ask price=%f, "
                    L"exchange sale conditions=%s, is trade=%s, type=%i\n",
                    tns[i].event_id, tns[i].time, tns[i].exchange_code, tns[i].price, tns[i].size,
                    tns[i].bid_price, tns[i].ask_price, tns[i].exchange_sale_conditions,
                    tns[i].is_trade ? L"True" : L"False", tns[i].type);
			fwprintf(output_stream,L"event id=%ld, time=%ld, exchange code=%c, price=%f, size=%li, bid price=%f, ask price=%f, "
                    L"exchange sale conditions=%s, is trade=%s, type=%i\n",
                    tns[i].event_id, tns[i].time, tns[i].exchange_code, tns[i].price, tns[i].size,
                    tns[i].bid_price, tns[i].ask_price, tns[i].exchange_sale_conditions,
                    tns[i].is_trade ? L"True" : L"False", tns[i].type);
        }
    }
}
/* -------------------------------------------------------------------------- */

void process_last_error () {
    int error_code = dx_ec_success;
    dxf_const_string_t error_descr = NULL;
    int res;

    res = dxf_get_last_error(&error_code, &error_descr);

    if (res == DXF_SUCCESS) {
        if (error_code == dx_ec_success) {
            printf("WTF - no error information is stored");

            return;
        }

        wprintf(L"Error occurred and successfully retrieved:\n"
            L"error code = %d, description = \"%s\"\n",
            error_code, error_descr);
        return;
    }

    printf("An error occurred but the error subsystem failed to initialize\n");
}
dxf_string_t ansi_to_unicode (const char* ansi_str) {
#ifdef _WIN32
    size_t len = strlen(ansi_str);
    dxf_string_t wide_str = NULL;

    // get required size
    int wide_size = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, ansi_str, (int)len, wide_str, 0);

    if (wide_size > 0) {
        wide_str = calloc(wide_size + 1, sizeof(dxf_char_t));
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, ansi_str, (int)len, wide_str, wide_size);
    }

    return wide_str;
#else /* _WIN32 */
    return NULL; /* todo */
#endif /* _WIN32 */
}

/* -------------------------------------------------------------------------- */

void terminate (int param)
{
  printf ("Terminating program...\n");
  exit(1);
}

int main (int argc, char* argv[]) {
    dxf_connection_t connection;
    dxf_subscription_t subscription;
    int loop_counter = 100; // *100 = msec of running
	int i = 0;
	int arg = 1;
	int line_size = 200;
	char line[200];
	char * pch;
	char** symbols;
	int symbols_pos = 0;
	int symbols_max = 10000;
	time_t start, end;
	int diff_time;
	int cmp;

	signal (SIGSEGV,terminate);

	output_stream = fopen("data.output", "w" );
	if (output_stream == NULL){
		printf("Couldn't open output file.");
		return -1;
	}

	symbols = (char*)malloc (symbols_max*sizeof(char*));
	if (argc > 1){ //we have params
		cmp = strcmp( argv[1] , "print" );
		if (!strcmp( argv[1] , "print" )){
			doPrint = true;
			++arg;
		}
		for (i = arg; i < argc ; ++i ){
			FILE *file;
			file = fopen (argv[i], "rt");
			if (file == NULL){
				printf("Couldn't open input file.");
				return -1;
			}
			//fgets(line, line_size, file); //skipping first line
			while(fgets(line, line_size, file) != NULL){		
				if (line[0] == '#')
					continue;
				pch = strtok (line,",");
			    pch = strtok (NULL, ",");// we need a second token
				if (symbols_pos < symbols_max){
					symbols[symbols_pos] = strdup(pch); 
					++symbols_pos;
				}
			}
			fclose(file);
		}
	}

    dxf_initialize_logger( "log.log", true, true, true );
    InitializeCriticalSection(&listener_thread_guard);
	
	printf("Sample test started.\n");    
    printf("Connecting to host %s...\n", dxfeed_host);
    
    if (!dxf_create_connection(dxfeed_host, on_reader_thread_terminate, NULL, NULL, NULL, &connection)) {
        process_last_error();
        return -1;
    }

    printf("Connection successful!\n");
 	time (&start);
	if (!dxf_create_subscription(connection, DXF_ET_TRADE /*| DXF_ET_QUOTE | DXF_ET_ORDER | DXF_ET_SUMMARY | DXF_ET_PROFILE*/, &subscription)) {
        process_last_error();
        
        return -1;
    };
	if (symbols_pos > 0){
		for (i = 0; i < symbols_pos; ++i){
			printf("Subscribing to: %s\n",symbols[i]);
			if (!dxf_add_symbol(subscription, ansi_to_unicode(symbols[i]))) {
				 process_last_error();
        
				 return -1;
			 }; 
		};
	};

	if (!dxf_attach_event_listener(subscription, listener, NULL)) {
        process_last_error();
        
        return -1;
    };
    printf("Subscription successful!\n");

    while (!is_thread_terminate() && loop_counter--) {
        Sleep(100);
    }
    DeleteCriticalSection(&listener_thread_guard);

    printf("Disconnecting from host...\n");
    
    if (!dxf_close_connection(connection)) {
        process_last_error();
        
        return -1;
    }
    time(&end);
	diff_time = (int)difftime(end, start);

	printf("Disconnect successful!\n"
           "Connection test completed successfully!\n");
           
	printf("received %i quotes in %i sec. %i qoutes in 1 sec\n", quotes_counter, diff_time, (int )(quotes_counter / diff_time));

	fclose (output_stream);
    return 0;
}
