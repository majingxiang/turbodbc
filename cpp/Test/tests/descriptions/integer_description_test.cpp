#include "turbodbc/descriptions/integer_description.h"

#include <gtest/gtest.h>
#include <sqlext.h>


TEST(IntegerDescriptionTest, BasicProperties)
{
	turbodbc::integer_description const description;

	EXPECT_EQ(sizeof(long), description.element_size());
	EXPECT_EQ(SQL_C_SBIGINT, description.column_c_type());
	EXPECT_EQ(SQL_BIGINT, description.column_sql_type());
}

TEST(IntegerDescriptionTest, MakeField)
{
	long const expected = 42;
	turbodbc::integer_description const description;

	EXPECT_EQ(turbodbc::field{expected}, description.make_field(reinterpret_cast<char const *>(&expected)));
}

TEST(IntegerDescriptionTest, SetField)
{
	long const expected = 42;
	turbodbc::integer_description const description;

	cpp_odbc::multi_value_buffer buffer(description.element_size(), 1);
	auto element = buffer[0];

	description.set_field(element, turbodbc::field{expected});
	EXPECT_EQ(expected, *reinterpret_cast<long *>(element.data_pointer));
	EXPECT_EQ(description.element_size(), element.indicator);
}

TEST(IntegerDescriptionTest, GetTypeCode)
{
	turbodbc::integer_description const description;
	EXPECT_EQ(turbodbc::type_code::integer, description.get_type_code());
}

TEST(IntegerDescriptionTest, CustomNameAndNullableSupport)
{
	std::string const expected_name("my_name");
	bool const expected_supports_null = false;

	turbodbc::integer_description const description(expected_name, expected_supports_null);

	EXPECT_EQ(expected_name, description.name());
	EXPECT_EQ(expected_supports_null, description.supports_null_values());
}
