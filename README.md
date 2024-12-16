# CodingExamples

Disclaimer: some elements of Rydbergs and CellularAutomata are taken directly from examples in MS documentation for windows.h, namely basewin.h which is present in each and virtually unmodified.

Examples of projects I've worked on for employers:

/FinanceStuff

Initially intended for backtesting a particular trading strategy I was interested in. In order to do this I programatically requested free OHLC data using the Alpaca API (DataCollector.py) and stored it using a custom file format (Utils.py) before backtesting and storing the results (DayReturnsObserver.py). After using this data to backtest the trading strategy I came to the conclusion that it didn't return well, as you would expect. I later used it to create a 200 by 200 matrix of how correlated the prices of 200 different stocks were over the period 2021 to 2023 which someone wanted to use for their dissertation.

/CellularAutomata

I simulate conways game of life and display it graphically using GDI from the windows.h library. I decided to work with the windows.h library in order to learn more about how my computer works behind the scenes. With regards to the logic behind the game of life simulation I use lists to keep track which cells are in the neighbourhood of cells that changed last generation in order to avoid updating regions which have reached stable configurations. It ran way faster than a friends version which he coded in python and took him half the time. Worth it!

/Rydbergs

I simulate ions, rydberg and groundstate atoms bouncing around in a box with an ionisation beam at one end and a rydberg-isation beam at the other. The Rydbergs promote groundstate atoms to rydbergs on contact. The ions promote rydbergs to ions on contact. The ions decay to groundstate atoms over time. I use chunking in order to efficiently calculate when collisions occur. I've since been told (by the student who's project I was assisting with) that the physics behind it is incorrect so it's really coloured balls bouncing around who's colours change according to arbitrary rules.

/Durhack2024 - We won an award for this.

I entered durhack with a group of friends - one of which is diabeteic - and we decided to analyse one of their blood glucose levels which they had data on spanning 3 years. My contribution was analysing their levels using markov chains where I broke up the blood glucose levels in to different regions (hypo, normal, low hyper etc...) and broke up the day into time chunks creating different transition matrixes between each time chunk. The motivations was to assess when he was most likely to enter hypos and hypers which we assumed were affected by the time of day (due to behaviors associated with those times) and glucose level in the previous levels (you'd assume if he's just had a massive hyper that he probably wouldn't be hypo-ing anytime soon). We then plotted these results and showed them to the represetntatives from Waterston's (the tech consultancy not the book shop) and they were sufficiently impressed to award us their prize.
