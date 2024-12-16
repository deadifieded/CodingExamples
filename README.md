# CodingExamples
Examples of projects I've worked on for employers

FinanceStuff
Initially intended for backtesting a particular trading strategy I was interested in. In order to do this I prgramatically requested free OHLC data using the Alpaca API () and stored it using a custom file format (Utils.py) before backtesting and storing the results (). After using this data to backtest the trading strategy I came to the conclusion that it didn't return well, as you would expect. I later used it to create a 200 by 200 matrix of how correlated the prices of 200 different stocks were over the period 2021 to 2023 which a friend wanted to use for his dissertation.

CellularAutomata
I simulate conways game of life and display it graphically using GDI in the windows.h library. I decided to work with the windows.h library in order to learn more about how my computer works behind the scenes. With regards to the logic behind the game of life simulation I use lists to keep track which cells are in the neighbourhood of cells that changed last generation in order to avoid updating regions which reached stable configurations. It ran way faster than a friends version which he coded in python and took him half the time. Worth it!

Durhack2024
I entered durhack with a group of friends - one of which is diabeteic - and we decided to analyse on of their blood glucose levels which they had data on spanning 3 years. My contribution was analysing they're levels using markov chains where we broke up the blood glucose levels in to different regions (hypo, normal, low hyper etc...) and broke up the day into time chunks where we had a different transition matrixes between each time chunk. The motivations was to assess when he was most likely to enter a hypos and hypers which we assumed were affected by time of day (due to behaviors associated with those times) and 
