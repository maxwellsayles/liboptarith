{-
Generate square free integers.  The solution used is taken directly
from http://oeis.org/A005117
-}
import Control.Arrow
import Data.List (intersperse, unfoldr)
import System.Environment (getArgs, getProgName)
import Text.Printf (printf)

primes :: [Integer]
primes = 2 : filter (\x -> all (\y -> x `mod` y /= 0) $
                           takeWhile (\y -> y*y <= x) primes) [3, 5..]

a000040_list = primes

a020639 n = spf a000040_list where
    spf (p:ps) | n < p^2      = n
               | mod n p == 0 = p
               | otherwise    = spf ps

a124010_row 1 = [0]
a124010_row n = unfoldr fact n where
    fact 1 = Nothing
    fact x = Just $ expo 0 x where
        p = a020639 x
        expo e x | m > 0     = (e, x)
                 | otherwise = expo (e+1) x' where (x', m) = divMod x p

a008966 = fromEnum . all (< 2) . a124010_row

a005117_list = filter ((== 1) . a008966) [1..]

square_frees = a005117_list

usage = do
  progName <- getProgName
  printf "Usage: %s <filename> <count>\n" progName
  printf "\n"
  printf "  <filename> should leave off the extension.\n"
  printf "  This program will produce both <filename.h> and <filename.c>.\n"
  printf "\n"

headerFile =
    unlines ["#pragma once",
             "#ifndef SQUARE_FREE__INCLUDED",
             "#define SQUARE_FREE__INCLUDED",
             "",
             "extern unsigned int square_free_count;",
             "extern unsigned int square_free[];",
             "",
             "#endif  // SQUARE_FREE__INCLUDED",
             ""]

formatOutput colSize xs =
    map (\l -> if last l == ' ' then init l else l) $
    groupLines [""] $
    concatPairs $
    intersperse ", " $
    map show xs
    where concatPairs [] = []
          concatPairs [x] = [x]
          concatPairs (x1:x2:xs) = (x1 ++ x2) : concatPairs xs

          groupLines rs [] = reverse rs
          groupLines (r:rs) (x:xs)
              | length r + length x > colSize = groupLines ([]:r:rs) (x:xs)
              | otherwise = groupLines ((r ++ x):rs) xs
        
main = do
  args <- getArgs
  if length args /= 2 then usage
  else do
    writeFile (args !! 0 ++ ".h") headerFile
    let ls = map ("  "++) $ formatOutput 78 $ take (read (args !! 1)) square_frees
    let source = unlines $
                 ["unsigned int square_free_count = " ++ args !! 1 ++ ";",
                  "unsigned int square_free[] = {"] ++
                 ls ++ 
                 ["};", ""]
    writeFile (args !! 0 ++ ".c") source
    putStrLn "Finished."

-- The below works, but isn't as time or as space efficient as the above.
{-
merge :: Ord a => [a] -> [a] -> [a]
merge [] ys = ys
merge xs [] = xs
merge (x:xs) (y:ys)
    | x < y     = x : xs `merge` (y:ys)
    | x > y     = y : (x:xs) `merge` ys
    | otherwise = x : xs `merge` ys

square_frees :: [Integer]
square_frees = 1 : loop primes []
    where loop (p:ps) xs =
              let as       = map (* p) xs
                  bs       = map (* p) $ takeWhile (< p) square_frees
                  cs       = xs `merge` as `merge` bs
                  (ys, zs) = span (<= p) cs
              in  ys ++ loop ps zs
-}               