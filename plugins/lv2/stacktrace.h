#pragma once

static std::string
symbol_demangle (const std::string& l)
{
	int status;

	try 
	{
		char* realname = abi::__cxa_demangle (l.c_str(), 0, 0, &status);
		std::string d (realname);
		free (realname);
		return d;
	} 
	catch (std::exception) 
	{

	}

	return l;
}
std::string demangle (std::string const & l)
{
	std::string::size_type const b = l.find_first_of ("(");

	if (b == std::string::npos) 
	{
		return symbol_demangle (l);
	}

	std::string::size_type const p = l.find_last_of ("+");
	if (p == std::string::npos) 
	{
		return symbol_demangle (l);
	}

	if ((p - b) <= 1) 
	{
		return symbol_demangle (l);
	}

	std::string const fn = l.substr (b + 1, p - b - 1);

	return symbol_demangle (fn);
}

void stacktrace (std::ostream& out, int levels)
{
	void *array[200];
	size_t size;
	char **strings;
	size_t i;
		
	size = backtrace (array, 200);

	if (size) 
	{
		strings = backtrace_symbols (array, size);
			
		if (strings) 
		{

			for (i = 0; i < size && (levels == 0 || i < size_t(levels)); i++) 
			{
				out << " " << demangle (strings[i]) << std::endl;
			}

			free (strings);
		}
	} 
	else 
	{
		out << "no stacktrace available!" << std::endl;
	}
}
