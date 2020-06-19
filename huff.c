#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#define AB_SIZE 256
typedef struct Node Node;
typedef struct Min_heap Min_heap;
typedef struct Huff_map Huff_map;
typedef struct Bit Bit;

enum Mode {
	code, decode
};

struct Huff_map {
	unsigned code[AB_SIZE];
	unsigned char size[AB_SIZE];
};

struct Bit {
	unsigned char value;
	unsigned char spot;
};

struct Node {
	unsigned char data;
	unsigned freq;
	Node* left;
	Node* right;
	Node* parent;
};

struct Min_heap {
	unsigned size;
	Node** array;
};

Node* create_node(unsigned char data, unsigned freq) {
	Node* node = malloc(sizeof(Node));
	if (node == NULL) {
		printf("memory error\n");
		return NULL;
	}
	node->data = data;
	node->freq = freq;
	node->left = NULL;
	node->right = NULL;
	node->parent = NULL;
	return node;
}

void swap_nodes(Node** a, Node** b) {
	Node* temp = *a;
	*a = *b;
	*b = temp;
}

bool cmp_nodes(Node* a, Node* b) {
	if (a->freq >= b->freq) return true;
	return false;
}

void arrange(Min_heap* min_heap, unsigned p_index) {
	unsigned s_index = p_index;
	unsigned l_index = 2 * p_index + 1;
	unsigned r_index = 2 * p_index + 2;
	if (l_index < min_heap->size && !cmp_nodes(min_heap->array[l_index], min_heap->array[s_index])) {
		s_index = l_index;
	}
	if (r_index < min_heap->size && !cmp_nodes(min_heap->array[r_index], min_heap->array[s_index])) {
		s_index = r_index;
	}
	if (s_index != p_index) {
		swap_nodes(&min_heap->array[s_index], &min_heap->array[p_index]);
		arrange(min_heap, s_index);
	}
}

void insert_node(Node* node, Min_heap* min_heap) {
	unsigned i = min_heap->size;
	min_heap->size++;
	while (i > 0 && !cmp_nodes(node, min_heap->array[(i - 1) / 2])) {
		min_heap->array[i] = min_heap->array[(i - 1) / 2];
		i = (i - 1) / 2;
	}
	min_heap->array[i] = node;
}

Node* pop_node(Min_heap* min_heap) {
	if (min_heap->size == 0) {
		return NULL;
	}
	min_heap->size--;
	Node* temp = min_heap->array[0];
	min_heap->array[0] = min_heap->array[min_heap->size];
	arrange(min_heap, 0);
	return temp;
}

void free_heap(Min_heap* min_heap, int i) {
	for (int k = 0; k < i; k++) {
		free(min_heap->array[k]);
	}
	free(min_heap->array);
	free(min_heap);
}

Min_heap* create_min_heap(unsigned* freq, unsigned size) {
	Min_heap* min_heap = malloc(sizeof(Min_heap));
	if (min_heap == NULL) {
		printf("memory error\n");
		return NULL;
	}
	min_heap->array = malloc(size * sizeof(Node*));
	if (min_heap->array == NULL) {
		free(min_heap);
		printf("memory error\n");
		return NULL;
	}
	min_heap->size = size;
	for (unsigned i = 0, j = 0; i < size; i++, j++) {
		while (j < AB_SIZE && freq[j] == 0) j++;
		min_heap->array[i] = create_node(j, freq[j]);
		if (min_heap->array[i] == NULL) {
			free_heap(min_heap, i);
			return NULL;
		}
	}
	for (int i = (min_heap->size - 1) / 2; i >= 0; i--) {
		arrange(min_heap, (unsigned)i);
	}
	return min_heap;
}

Node* build_huffman_tree_from_heap(Min_heap* min_heap) {
	Node* root = NULL;
	if (min_heap->size == 1) {
		root = pop_node(min_heap);
	}
	while (min_heap->size > 1) {
		Node* left = pop_node(min_heap);
		Node* right = pop_node(min_heap);
		root = create_node('0', left->freq + right->freq);
		if (root == NULL) {
			free_heap(min_heap, min_heap->size);
			return NULL;
		}
		root->left = left;
		root->right = right;
		insert_node(root, min_heap);
	}
	pop_node(min_heap);
	free_heap(min_heap, 0);
	return root;
}

void free_tree(Node* node) {
	if (node != NULL) {
		free_tree(node->right);
		free_tree(node->left);
		free(node);
	}
}

void build_freq(unsigned* freq, FILE* fin, int shift, unsigned* size) {
	for (int i = 0; i < AB_SIZE; i++) {
		freq[i] = 0;
	}
	int c = fgetc(fin);
	while (c != EOF) {
		if (freq[c] == 0) {
			(*size)++;
		}
		freq[c]++;
		c = fgetc(fin);
	}
	fseek(fin, shift, SEEK_SET);
}

void assign(Node* node, Huff_map* huff_map, unsigned char size, unsigned code) {
	if (node != NULL) {
		if (node->left == NULL && node->right == NULL) {
			huff_map->code[node->data] = code;
			huff_map->size[node->data] = size;
		}
		else {
			assign(node->left, huff_map, size + 1, code << 1);
			assign(node->right, huff_map, size + 1, (code << 1) + 1);
		}
	}
}

Huff_map* create_huff_map(Node* root) {
	Huff_map* huff_map = malloc(sizeof(Huff_map));
	if (huff_map == NULL) {
		printf("memory error\n");
		free_tree(root);
		return NULL;
	}
	for (int i = 0; i < AB_SIZE; i++) {
		huff_map->code[i] = 0;
		huff_map->size[i] = 0;
	}
	if (root->left == NULL && root->right == NULL) {
		huff_map->size[root->data] = 1;
	}
	else {
		unsigned char size = 0;
		unsigned code = 0;
		assign(root, huff_map, size, code);
	}
	return huff_map;
}

Bit* create_bit() {
	Bit* bit = malloc(sizeof(Bit));
	if (bit == NULL) {
		printf("memory error\n");
		return NULL;
	}
	bit->value = 128;
	bit->spot = 8;
	return bit;
}

void bit_pack(unsigned char size, unsigned code, unsigned char* byte, Bit* bit, FILE* fout) {
	while (size) {
		unsigned temp_code = 0;
		unsigned char temp_size = 0;
		if (bit->spot > size) {
			temp_code = code << (bit->spot - size);
			temp_size = size;
		}
		else {
			temp_code = code >> (size - bit->spot);
			temp_size = bit->spot;
		}
		for (int i = 0; i < temp_size; i++) {
			*byte += temp_code & bit->value;
			bit->spot--;
			bit->value = bit->value >> 1;
		}
		size -= temp_size;
		if (bit->value == 0) {
			bit->value = 128;
			bit->spot = 8;
			fwrite(byte, sizeof(unsigned char), 1, fout);
			*byte = 0;
		}
	}
}



bool save_to_huff(Huff_map* huff_map, FILE* fin, FILE* fout) {
	int c = fgetc(fin);
	Bit* bit = create_bit();
	if (bit == NULL) {
		return false;
	}
	unsigned char byte = 0;
	while (c != EOF) {
		unsigned code = huff_map->code[c];
		unsigned char size = huff_map->size[c];
		bit_pack(size, code, &byte, bit, fout);
		c = fgetc(fin);
	}
	if (bit->value != 128) {
		fwrite(&byte, sizeof(unsigned char), 1, fout);
		fwrite(&bit->spot, sizeof(unsigned char), 1, fout);
	}
	else {
		unsigned char ch = 0;
		fwrite(&ch, sizeof(unsigned char), 1, fout);
	}
	free(bit);
	return true;
}

void close_files(FILE* fp1, FILE* fp2) {
	fclose(fp1);
	fclose(fp2);
}

Node* byte_dec(unsigned char bit, unsigned char byte, Node* node, Node* root, FILE* fout) {
	while (bit != 0) {
		if (node->left != NULL || node->right != NULL) {
			if (byte & bit) {
				node = node->right;
			}
			else {
				node = node->left;
			}
			bit = bit >> 1;
		}
		else {
			fwrite(&node->data, sizeof(unsigned char), 1, fout);
			node = root;
		}
	}
	if (node->left == NULL && node->right == NULL) {
		fwrite(&node->data, sizeof(unsigned char), 1, fout);
		node = root;
	}
	return node;
}

void save_back(Node* root, FILE* fin, FILE* fout, unsigned shift) {
	fseek(fin, -1, SEEK_END);
	char last_zeros = fgetc(fin);
	fseek(fin, -1, SEEK_CUR);
	unsigned pre_end = ftell(fin);
	fseek(fin, shift, SEEK_SET);
	Node* node = root;
	unsigned char byte = fgetc(fin);
	if (root->left == 0 && root->right == 0) {
		for (unsigned i = shift + 1; i < pre_end; i++) {
			for (int j = 0; j < 8; j++) {
				fwrite(&root->data, sizeof(unsigned char), 1, fout);
			}
		}
		for (int j = 0; j < 8 - last_zeros; j++) {
			fwrite(&root->data, sizeof(unsigned char), 1, fout);
		}
	}
	else {
		unsigned char bit = 128;
		for (unsigned i = shift + 1; i < pre_end; i++) {
			node = byte_dec(bit, byte, node, root, fout);
			byte = fgetc(fin);
		}
		byte = byte >> last_zeros;
		bit = bit >> last_zeros;
		node = byte_dec(bit, byte, node, root, fout);
	}
}

Bit* write_tree(Node* node, FILE* fout, Bit* bit, unsigned char* byte) {
	if (node != NULL) {
		if (node->left == NULL && node->right == NULL) {
			*byte += bit->value;
			bit->spot--;
			bit->value = bit->value >> 1;
			if (bit->value == 0) {
				fwrite(byte, sizeof(unsigned char), 1, fout);
				bit->spot = 8;
				bit->value = 128;
				*byte = 0;
			}
			bit_pack(8, node->data, byte, bit, fout);
		}
		else {
			bit->spot--;
			bit->value = bit->value >> 1;
			if (bit->value == 0) {
				fwrite(byte, sizeof(unsigned char), 1, fout);
				bit->spot = 8;
				bit->value = 128;
				*byte = 0;
			}
			bit = write_tree(node->left, fout, bit, byte);
			bit = write_tree(node->right, fout, bit, byte);
		}
	}
	return bit;
}

bool save_tree(Node* root, FILE* fout) {
	Bit* bit = create_bit();
	if (bit == NULL) {
		return false;
	}
	unsigned char byte = 0;
	bit = write_tree(root, fout, bit, &byte);
	if (bit->value != 128) {
		fwrite(&byte, sizeof(unsigned char), 1, fout);
	}
	return true;
}

Node* load_tree(FILE* fin) {
	unsigned char byte = fgetc(fin);
	Node* root = create_node('0', 0);
	if (root == NULL) {
		return NULL;
	}
	if ((byte & 128) != 0) {
		root->data = byte;
		root->data = root->data << 1;
		byte = fgetc(fin);
		byte = byte >> 7;
		root->data += byte & 1;
		return root;
	}
	Node* nodeD = root;
	Node* nodeU = root;
	Bit* bit = create_bit();
	if (bit == NULL) {
		printf("memory error\n");
		return NULL;
	}
	bool is_break = false;
	while (!is_break) {
		while ((byte & bit->value) == 0 && bit->spot > 0) {
			nodeD = create_node('0', 0);
			if (nodeD == NULL) {
				return NULL;
			}
			nodeD->parent = nodeU;
			nodeU->left = nodeD;
			nodeU = nodeD;
			bit->value = bit->value >> 1;
			bit->spot--;
		}
		if ((byte & bit->value) != 0 && nodeU == nodeU->parent->left) {
			nodeU = nodeD->parent;
			bit->value = bit->value >> 1;
			bit->spot--;
			if (bit->spot == 0) {
				byte = fgetc(fin);
				bit->value = 128;
				bit->spot = 8;
			}
			char shift = 8 - bit->spot;
			unsigned char data = byte << shift;
			byte = fgetc(fin);
			data += byte >> bit->spot;
			nodeD->data = data;
			nodeU->right = create_node('0', 0);
			if (nodeU->right == NULL) {
				return NULL;
			}
			nodeD = nodeU->right;
			nodeD->parent = nodeU;
			nodeU = nodeD;
		}
		else if ((byte & bit->value) != 0) {
			nodeU = nodeD->parent;
			bit->value = bit->value >> 1;
			bit->spot--;
			if (bit->spot == 0) {
				byte = fgetc(fin);
				bit->value = 128;
				bit->spot = 8;
			}
			char shift = 8 - bit->spot;
			unsigned char data = byte << shift;
			byte = fgetc(fin);
			data += byte >> bit->spot;
			nodeD->data = data;
			while (nodeU->parent != NULL && nodeU->parent->right == nodeU) {
				nodeD = nodeU;
				nodeU = nodeU->parent;
			}
			if (nodeU->parent != NULL) {
				nodeU = nodeU->parent;
				nodeD = create_node('0', 0);
				if (nodeD == NULL) {
					return NULL;
				}
				nodeU->right = nodeD;
				nodeD->parent = nodeU;
				nodeU = nodeD;
			}
			else {
				is_break = true;
				continue;
			}
		}
		if (bit->spot == 0) {
			byte = fgetc(fin);
			bit->value = 128;
			bit->spot = 8;
		}
	}
	free(bit);
	return root;
}

void print_tree(Node* node, int r) {
	if (node != NULL)
	{
		print_tree(node->right, r + 5);
		for (int i = 0; i < r; i++)
			printf(" ");
		printf("%c\n", node->data);
		print_tree(node->left, r + 5);
	}
}

bool check_size(FILE* fin, FILE* fout, int shift) {
	fseek(fin, 0, SEEK_END);
	int fsize = ftell(fin);
	if (fsize - shift <= 0) {
		close_files(fin, fout);
		return false;
	}
	fseek(fin, shift, SEEK_SET);
	return true;
}

bool code_file(FILE* fin, FILE* fout, int shift) {
	unsigned freq[AB_SIZE];
	unsigned size = 0;
	build_freq(freq, fin, shift, &size);
	Min_heap* min_heap = create_min_heap(freq, size);
	if (min_heap == NULL) {
		close_files(fin, fout);
		return false;
	}
	Node* root = build_huffman_tree_from_heap(min_heap);
	if (root == NULL) {
		close_files(fin, fout);
		return false;
	}
	Huff_map* huff_map = create_huff_map(root);
	if (huff_map == NULL) {
		close_files(fin, fout);
		return false;
	}
	if (!save_tree(root, fout)) {
		return false;
	}
	if (!save_to_huff(huff_map, fin, fout)) {
		printf("Can't save\n");
	}
	free_tree(root);
	free(huff_map);
	return true;
}

bool decode_file(FILE* fin, FILE* fout) {
	Node* root = load_tree(fin);
	if (root == NULL) {
		close_files(fin, fout);
		return false;
	}
	save_back(root, fin, fout, ftell(fin));
	free_tree(root);
	return true;
}

int main(int argc, char* argv[]) {
	FILE* fin;
	FILE* fout;
	int shift = 0;
	enum Mode mode = code;
	if (argc == 4) {
		fin = fopen(argv[2], "rb");
		if (fin == NULL) {
			printf("Can't open file\n");
			return 1;
		}
		fout = fopen(argv[3], "wb");
		if (fout == NULL) {
			printf("Can't open file\n");
			fclose(fin);
			return 1;
		}
		if (strcmp("-d", argv[1]) == 0) {
			mode = decode;
		}
		else if (strcmp("-c", argv[1]) != 0) {
			printf("type '-c' if you want to code or '-d' if you want to decode a file\n");
			close_files(fin, fout);
			return 1;
		}
	}
	else {
		fin = fopen("in.txt", "rb");
		if (fin == NULL) {
			printf("Can't open file\n");
			return 1;
		}
		fout = fopen("out.txt", "wb");
		if (fout == NULL) {
			printf("Can't open file\n");
			fclose(fin);
			return 1;
		}
		shift = 3;
		char m = fgetc(fin);
		if (m == 'd') {
			mode = decode;
		}
		else if (m != 'c') {
			printf("wrong input\n");
			close_files(fin, fout);
			return 1;
		}
	}
	if (!check_size(fin, fout, shift)) {
		return 0;
	}
	if (mode == code) {
		if (!code_file(fin, fout, shift)) {
			return 1;
		}
	}
	else {
		if (!decode_file(fin, fout)) {
			return 1;
		}
	}
	close_files(fin, fout);
	return 0;
}
