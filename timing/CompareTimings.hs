import Control.Applicative

loadFile :: FilePath -> IO [(Int, Double)]
loadFile filename =
    map read .
        map (\s -> "(" ++ s ++ ")") . lines <$>
        readFile filename

compareFiles f1 f2 dropCount = do
  dat1 <- drop dropCount . map snd <$> loadFile f1
  dat2 <- drop dropCount . map snd <$> loadFile f2
  let ratios = zipWith (/) dat1 dat2
  return $ (sum ratios) / (fromIntegral (length ratios))