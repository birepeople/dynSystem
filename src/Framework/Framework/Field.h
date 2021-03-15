#pragma once
#include <iostream>
#include "FieldBase.h"
#include "Base.h"
#include "Array/Array.h"
#include "Framework/Log.h"

namespace dyno {

/*!
*	\class	Variable
*	\brief	Variables of build-in data types.
*/
template<typename T>
class VarField : public FieldBase
{
public:
	typedef T VarType;
	typedef T DataType;
	typedef VarField<T> FieldClass;

	DEFINE_FIELD_FUNC(FieldClass, DataType);

	VarField();
	VarField(std::string name, std::string description, EFieldType fieldType, Base* parent);
	VarField(T value, std::string name, std::string description, EFieldType fieldType, Base* parent);
	~VarField() override;

	size_t getElementCount() override { return 1; }
	const std::string getTemplateName() override { return std::string(typeid(T).name()); }
	const std::string getClassName() override { return std::string("Variable"); }

	void setValue(T val);
};

template<typename T>
VarField<T>::VarField()
	: FieldBase("", "")
{
}

template<typename T>
VarField<T>::VarField(std::string name, std::string description, EFieldType fieldType, Base* parent)
	: FieldBase(name, description, fieldType, parent)
	, m_data(nullptr)
{
}

template<typename T>
VarField<T>::VarField(T value, std::string name, std::string description, EFieldType fieldType, Base* parent)
	: FieldBase(name, description, fieldType, parent)
{
	this->setValue(value);
}

template<typename T>
VarField<T>::~VarField()
{
};

template<typename T>
void VarField<T>::setValue(T val)
{
	std::shared_ptr<T>& data = this->getReference();
	if (data == nullptr)
	{
		data = std::make_shared<T>(val);
	}
	else
	{
		*data = val;
	}

	this->update();
}

template<typename T>
using HostVarField = VarField<T>;

template<typename T>
using DeviceVarField = VarField<T>;

template<typename T>
using HostVariablePtr = std::shared_ptr< HostVarField<T> >;

template<typename T>
using DeviceVariablePtr = std::shared_ptr< DeviceVarField<T> >;


/**
 * Define field for Array
 */
template<typename T, DeviceType deviceType>
class ArrayField : public FieldBase
{
public:
	typedef T VarType;
	typedef Array<T, deviceType> DataType;
	typedef ArrayField<T, deviceType> FieldClass;

	DEFINE_FIELD_FUNC(FieldClass, DataType);

	ArrayField();
	ArrayField(int num, std::string name, std::string description, EFieldType fieldType, Base* parent);
	~ArrayField() override;

	inline size_t getElementCount() override {
		auto ref = this->getReference();
		return ref == nullptr ? 0 : ref->size();
	}

	void setElementCount(size_t num);

	const std::string getTemplateName() override { return std::string(typeid(T).name()); }
	const std::string getClassName() override { return std::string("ArrayBuffer"); }

	void setValue(std::vector<T>& vals);
	void setValue(GArray<T>& vals);
};

template<typename T, DeviceType deviceType>
ArrayField<T, deviceType>::ArrayField()
	: FieldBase("", "")
{
}

template<typename T, DeviceType deviceType>
ArrayField<T, deviceType>::ArrayField(int num, std::string name, std::string description, EFieldType fieldType, Base* parent)
	: FieldBase(name, description, fieldType, parent)
{
	m_data = num <= 0 ? nullptr : std::make_shared<Array<T, deviceType>>(num);
}

template<typename T, DeviceType deviceType>
ArrayField<T, deviceType>::~ArrayField()
{
	if (m_data.use_count() == 1)
	{
		m_data->clear();
	}
}

template<typename T, DeviceType deviceType>
void ArrayField<T, deviceType>::setElementCount(size_t num)
{
	FieldBase* topField = this->getTopField();
	ArrayField<T, deviceType>* derived = dynamic_cast<ArrayField<T, deviceType>*>(topField);

	if (derived->m_data == nullptr)
	{
		derived->m_data = std::make_shared<Array<T, deviceType>>(num);
	}
	else
	{
		derived->m_data->resize(num);
	}
}

template<typename T, DeviceType deviceType>
void ArrayField<T, deviceType>::setValue(std::vector<T>& vals)
{
	std::shared_ptr<Array<T, deviceType>>& data = this->getReference();
	if (data == nullptr)
	{
		data = std::make_shared<Array<T, deviceType>>();
	}

	data->assign(vals);
}

template<typename T, DeviceType deviceType>
void ArrayField<T, deviceType>::setValue(GArray<T>& vals)
{
	std::shared_ptr<Array<T, deviceType>>& data = this->getReference();
	if (data == nullptr)
	{
		data = std::make_shared<Array<T, deviceType>>();
	}

	data->assign(vals);
}

template<typename T>
using HostArrayField = ArrayField<T, DeviceType::CPU>;

template<typename T>
using DeviceArrayField = ArrayField<T, DeviceType::GPU>;
}