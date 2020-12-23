#pragma once
#include <string>

class __declspec(dllexport) Address {
public:
	Address(std::string _tel,
		std::string _surname,
		std::string _name,
		std::string _fatherName,
		std::string _street,
		int _house,
		int _corp,
		int _flat)
	{
		tel = _tel;
		surname = _surname;
		name = _name;
		fatherName = _fatherName;
		street = _street;
		house = _house;
		corp = _corp;
		flat = _flat;
	}
	std::string tel;
	std::string surname;
	std::string name;
	std::string fatherName;
	std::string street;
	int house;
	int corp;
	int flat;
};

struct node
{
	int key;
	Address* adr;
	unsigned char height;
	node* left;
	node* right;
	node(int k)
	{
		key = k;
		left = right = 0;
		height = 1;
	}
};