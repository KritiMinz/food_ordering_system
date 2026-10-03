function Navbar({
  cartItemCount,
  onLogin,
  currentUser,
  onLogout,
  onProfile,
}) {
  const scrollToSection = (sectionId) => {
    const section = document.getElementById(sectionId)

    if (section) {
      section.scrollIntoView({
        behavior: "smooth",
        block: "start",
      })
    }
  }

  return (
    <nav className="bg-white shadow-md sticky top-0 z-40">

      <div className="max-w-7xl mx-auto px-6 py-4 flex items-center justify-between">

        {/* LOGO */}

        <button
          type="button"
          onClick={() => scrollToSection("home")}
          className="text-2xl font-bold text-orange-600"
        >
          🍔 FoodHub
        </button>


        {/* NAVIGATION */}

        <div className="flex items-center gap-8 text-gray-700">

          <button
            type="button"
            onClick={() => scrollToSection("home")}
            className="hover:text-orange-600"
          >
            Home
          </button>

          <button
            type="button"
            onClick={() => scrollToSection("menu")}
            className="hover:text-orange-600"
          >
            Menu
          </button>

          <button
            type="button"
            onClick={() => scrollToSection("orders")}
            className="hover:text-orange-600"
          >
            Orders
          </button>

          <button
            type="button"
            onClick={() => scrollToSection("cart")}
            className="hover:text-orange-600 flex items-center"
          >
            Cart 🛒

            {cartItemCount > 0 && (
              <span className="ml-1 bg-orange-600 text-white text-xs
                               px-2 py-1 rounded-full">
                {cartItemCount}
              </span>
            )}
          </button>


          {/* USER */}

          {currentUser ? (

            <div className="flex items-center gap-3">

              <span className="font-semibold text-gray-700">
                👤 {currentUser.name}
              </span>

              <button
                type="button"
                onClick={onProfile}
                className="border border-gray-300 text-gray-700
                           px-4 py-2 rounded-lg
                           hover:bg-gray-50"
              >
                Profile
              </button>

              <button
                type="button"
                onClick={onLogout}
                className="border border-orange-600 text-orange-600
                           px-4 py-2 rounded-lg
                           hover:bg-orange-50"
              >
                Logout
              </button>

            </div>

          ) : (

            <button
              type="button"
              onClick={onLogin}
              className="bg-orange-600 text-white px-5 py-2 rounded-lg
                         hover:bg-orange-700"
            >
              Login
            </button>

          )}

        </div>

      </div>

    </nav>
  )
}

export default Navbar

