@echo off
rem openssl zlib -e <_text_source >tmp_text.enc
rem openssl zlib -d <tmp_text.enc >tmp_text.dec

echo.
echo.
echo.
copy _text_source tmp_text
gzip -9 tmp_text

echo.
echo.
echo.
degzip -r tmp_text.gz tmp_text_deflate

echo.
echo.
echo.
degzip -u tmp_text.gz tmp_text_unc

echo.
echo.
echo.
pause

