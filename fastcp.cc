#include <cstdio>
#include <string>

using namespace std;

#include <sys/stat.h>

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "usage: %s <filename> [<filename> ..]\n\ncopies all listed files to the current directory.\n", argv[0]);
    return 0;
  }

  for (int i=1; i < argc; i++)
  {
    string srcfilename = argv[i];

    string::size_type last_slash = srcfilename.find_last_of("/");

    if (last_slash == string::npos)
    {
      fprintf(stderr, "%s: %s: file is already in current directory\n", argv[0], srcfilename.c_str());
      continue;
    }

    string dstfilename = srcfilename.substr(last_slash + 1);

    struct stat st;
    char *open_mode = "wb";

    int result = stat(dstfilename.c_str(), &st);

    if (result == 0)
      open_mode = "rb+";

    FILE *in, *out;

    in = fopen(srcfilename.c_str(), "rb");
    if (in == NULL)
    {
      fprintf(stderr, "%s: %s: unable to open file\n", argv[0], srcfilename.c_str());
      continue;
    }

    out = fopen(dstfilename.c_str(), open_mode);
    if (out == NULL)
    {
      fclose(in);
      fprintf(stderr, "%s: ./%s: unable to open file\n", argv[0], dstfilename.c_str());
      continue;
    }

    fseek(in, 0, SEEK_END);
    int size = ftell(in);

    fseek(out, 0, SEEK_END);
    int offset = ftell(out);

    if (offset == size)
    {
      fprintf(stderr, "%s: %s: already copied\n", argv[0], dstfilename.c_str());
      fclose(in);
      fclose(out);
      continue;
    }

    if (offset > 0)
      fprintf(stderr, "%s: %s: resuming from byte offset %d\n", argv[0], dstfilename.c_str(), offset);

    fseek(in, offset, SEEK_SET);

    fseek(out, size - 1, SEEK_SET);
    fputc(0, out);
    fseek(out, offset, SEEK_SET);

    int bytes_remaining = size - offset;
    char *buf = new char[131072];
    while (bytes_remaining > 0)
    {
      int bytes_this_block = 131072;
      if (bytes_this_block > bytes_remaining)
        bytes_this_block = bytes_remaining;

      int read = fread(buf, 1, bytes_this_block, in);

      if (read <= 0)
      {
        fprintf(stderr, "%s: %s: read error at offset %d\n", argv[0], dstfilename.c_str(), ftell(in));
        break;
      }

      fwrite(buf, 1, read, out);

      bytes_remaining -= read;
    }

    fclose(in);
    fclose(out);
  }
}
