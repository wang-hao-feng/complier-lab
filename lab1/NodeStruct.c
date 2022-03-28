struct Node
{
    struct Node *left;
    struct Node *right;
    union
    {
        unsigned int type_int;
        double type_float;
        char *type_string;
    }val;
    int val_type;
    int line;
    char *type;
};