unsigned int swap_endian_int(unsigned int num)
{
	return (num >> 24) | ((num << 8) & 0x00FF0000) | ((num >> 8) & 0x0000FF00) | (num << 24);
}

unsigned short swap_endian_short(unsigned short num)
{
	return (num >> 8) | (num << 8);
}
